#pragma once

#include "stacktrace.h"
#include <ctype.h>
#include <math.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "sai_string.h"
#include "libtcc.h"

char *log_buffer;
FILE *log_stream;
int log_on = 0;
size_t log_size;

#ifdef COMPILED_WITH_TCC
int tcc_backtrace(const char *fmt, ...);
#define log123(...) \
    fprintf(log_stream, __VA_ARGS__)
#define prn(...) \
    printf(__VA_ARGS__)

#define log(...)                                    \
    {                                               \
        printf("LOG: %s:%d: ", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                        \
    }

#else

#define log(...)                                \
    printf("LOG: %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__)

#define prn(...) \
    printf(__VA_ARGS__)

#ifdef VAJER_EVAL
#define tcc_backtrace printf
#else
int tcc_backtrace(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    printStackTrace(0);
    assert(0);
    return 0;
}
#endif
#endif

void dump_log()
{
    fflush(log_stream);
    printf("\n\e[32m# log\e[0m\n%s", log_buffer);
}

void clear_log()
{
    fclose(log_stream);
    free(log_buffer);
    log_buffer = NULL;
    log_stream = open_memstream(&log_buffer, &log_size);
}

#define sai_assert(cb)                                          \
    if (!(cb))                                                  \
    {                                                           \
        dump_log();                                             \
        printf("\033[1;31m%s:%d\033[0m\n", __FILE__, __LINE__); \
        tcc_backtrace("\e[31m" #cb "\e[m");                     \
        exit(1);                                                \
    }

typedef enum TokenType
{
    TOKEN_OPEN_PARENS,
    TOKEN_CLOSE_PARENS,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_OPEN_BRACES,
    TOKEN_CLOSE_BRACES,

    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_NUMBER,

    TOKEN_WHITESPACE,
    TOKEN_NEWLINE,
    TOKEN_COMMENT,
} TokenType;

char *token_names[] = {
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    "\"",
    "SYMBOL",
    "NUMBER",
    "WHITESPACE",
    "NEWLINE",
    "COMMENT",
};

typedef struct Token
{
    TokenType type;
    int start;
    int stop;
} Token;

typedef struct TokenizeState
{
    const char *code;
    int count;
    int pos;
} TokenizeState;

Token whitespace(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && (state->code[state->pos] == ' ' || state->code[state->pos] == ','))
        state->pos++;
    return (Token){TOKEN_WHITESPACE, start, state->pos};
}

Token newline(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && state->code[state->pos] == '\n')
        state->pos++;
    return (Token){TOKEN_NEWLINE, start, state->pos};
}

const char *symbol_chars = "<>=*-?+:_!&.@'\\%|/";

int is_symbol(int is_first, char c)
{
    return (is_first ? isalpha(c) : isalpha(c) || isdigit(c)) || strchr(symbol_chars, c) != NULL;
}

Token token_symbol(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && is_symbol(start == state->pos, state->code[state->pos]))
        state->pos++;
    return (Token){TOKEN_SYMBOL, start, state->pos};
}

Token number(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && isdigit(state->code[state->pos]))
        state->pos++;
    return (Token){TOKEN_NUMBER, start, state->pos};
}

Token negative_number_or_symbol(TokenizeState *state)
{
    int start = state->pos;
    if (state->pos + 1 < state->count && isdigit(state->code[state->pos + 1]))
    {
        state->pos++;
        while (state->pos < state->count && isdigit(state->code[state->pos]))
            state->pos++;
        return (Token){TOKEN_NUMBER, start, state->pos};
    }
    else
    {
        return token_symbol(state);
    }
}

Token token_string(TokenizeState *state)
{
    int start = state->pos;
    state->pos++;
    int escape = 0;
    while (state->pos < state->count)
    {
        char c = state->code[state->pos];
        if (escape)
        {
            escape = 0;
            state->pos++;
        }
        else if (c == '\\')
        {
            escape = 1;
            state->pos++;
        }
        else if (c == '"')
        {
            state->pos++;
            break;
        }
        else
        {
            state->pos++;
        }
    }
    return (Token){TOKEN_STRING, start, state->pos};
}

Token comment(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && state->code[state->pos] != '\n')
        state->pos++;
    return (Token){TOKEN_COMMENT, start, state->pos};
}

Token *tokenize(const char *str, int count)
{
    TokenizeState state = {str, count, 0};
    Token *tokens = NULL;

    while (state.pos < state.count)
    {
        char c = state.code[state.pos];
        if (c == '(')
        {
            arrpush(tokens, ((Token){.type = TOKEN_OPEN_PARENS, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == ')')
        {
            arrpush(tokens, ((Token){.type = TOKEN_CLOSE_PARENS, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == '[')
        {
            arrpush(tokens, ((Token){.type = TOKEN_OPEN_BRACKET, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == ']')
        {
            arrpush(tokens, ((Token){.type = TOKEN_CLOSE_BRACKET, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == '{')
        {
            arrpush(tokens, ((Token){.type = TOKEN_OPEN_BRACES, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == '}')
        {
            arrpush(tokens, ((Token){.type = TOKEN_CLOSE_BRACES, .start = state.pos, .stop = state.pos + 1}));
            state.pos++;
        }
        else if (c == ' ' || c == ',')
        {
            arrpush(tokens, whitespace(&state));
        }
        else if (c == '#')
        {
            arrpush(tokens, comment(&state));
        }
        else if (c == '"')
        {
            arrpush(tokens, token_string(&state));
        }
        else if (c == '\n')
        {
            arrpush(tokens, newline(&state));
        }
        else if (isdigit(c))
        {
            arrpush(tokens, number(&state));
        }
        else if (c == '-')
        {
            arrpush(tokens, negative_number_or_symbol(&state));
        }
        else if (is_symbol(1, c))
        {
            arrpush(tokens, token_symbol(&state));
        }
        else
        {
            log("\033[0;31munhandled: %c\033[0m\n", c);
            sai_assert(0);
            // state.pos++;
        }
    }

    return tokens;
}

////////////////// Parsing //////////////////////

typedef enum ASTType
{
    AST_LIST = 1,

    AST_MAP,

    AST_STRING,
    AST_SYMBOL,
    AST_NUMBER,
    AST_BOOLEAN,

    AST_EMPTY,
} ASTType;

typedef enum ListType
{
    LIST_PARENS,
    LIST_BRACKETS,
} ListType;

TokenType closing_types[] = {
    TOKEN_CLOSE_PARENS,
    TOKEN_CLOSE_BRACKET,
};

struct Ast;
struct AstKV;

typedef struct List
{
    ListType type;
    struct AST *elements;
} List;

typedef struct Map
{
    struct AstKV *kvs;
} Map;

typedef struct SourceMapping
{
    int start;
    int stop;
} SourceMapping;

typedef struct AST
{
    ASTType ast_type;
    SourceMapping source;
    int no_semicolon;
    struct AST *value_type;
    union
    {
        List list;
        Map map;
        char *string;
        char *symbol;
        int number;
        int boolean;
    };
} AST;

typedef struct AstKV
{
    AST key;
    AST value;
} AstKV;

typedef struct TypeKV
{
    AST *key;
    AST *value;
} TypeKV;

typedef struct ParseState
{
    const char *code;
    Token *tokens;
    int pos;
} ParseState;

typedef struct PrintASTState
{
    int indentation;
    int dark;
    int line_len;
} PrintASTState;

char *ansi_blue = "\e[0;34m";
char *ansi_clear = "\e[0m";

char *ansi_indent_color[] = {
    "\e[0;30m",
    "\e[0;31m",
    "\e[0;32m",
    "\e[0;33m",
    "\e[0;34m",
    "\e[0;35m",
    "\e[0;36m",
    "\e[0;37m",
};

char *ansi_indent_color_bg[] = {
    "\e[0;40m",
    "\e[0;41m",
    "\e[0;42m",
    "\e[0;43m",
    "\e[0;44m",
    "\e[0;45m",
    "\e[0;46m",
    "\e[0;47m",
};

char *ansi_dark_background = "\e[100m";
char *ansi_bright_background = "\e[101m";

void print_indent(int indent)
{
    int dark = 0;
    for (int i = 0; i < indent; i++)
    {
        dark = !dark;
        if (dark)
        {
            prn("%s", ansi_dark_background);
        }
        else
        {
            prn("%s", ansi_bright_background);
        }

        prn("%s", ansi_indent_color[i]);
        prn(" ");
    }
}

void _print_ast(PrintASTState *state, AST *el)
{
    switch (el->ast_type)
    {
    case AST_SYMBOL:
        prn("%s", el->symbol);
        break;
    case AST_STRING:
        prn("\"%s\"", el->string);
        break;
    case AST_NUMBER:
        prn("%d", el->number);
        break;
    case AST_BOOLEAN:
        prn("%s", el->boolean == 0 ? "false" : "true");
        break;
    case AST_LIST:
        prn("%s", ansi_indent_color[state->indentation]);
        if (el->list.type == LIST_PARENS)
        {
            prn("(");
        }
        else if (el->list.type == LIST_BRACKETS)
        {
            prn("[");
        }
        prn("%s", ansi_clear);

        if (arrlen(el->list.elements) > 4)
        {
            state->line_len = 0;
            state->indentation += 1;

            for (int i = 0; i < arrlen(el->list.elements); i++)
            {

                state->dark = !state->dark;

                if (state->dark)
                {
                    prn("%s", ansi_dark_background);
                }
                else
                {
                    prn("%s", ansi_bright_background);
                }

                if (i > 0)
                    print_indent(state->indentation);

                prn("%s", ansi_clear);

                state->line_len = 0;
                _print_ast(state, &el->list.elements[i]);

                if (i < arrlen(el->list.elements) - 0)
                {
                    prn("\n");
                }
            }

            state->indentation -= 1;
        }
        else
        {
            state->line_len += arrlen(el->list.elements);

            for (int i = 0; i < arrlen(el->list.elements); i++)
            {
                _print_ast(state, &el->list.elements[i]);

                if (i < arrlen(el->list.elements) - 1)
                {
                    prn(" ");
                }
            }
        }

        if (state->dark)
        {
            prn("%s", ansi_dark_background);
        }
        else
        {
            prn("%s", ansi_bright_background);
        }

        // print_indent(state->indentation);

        prn("%s", ansi_clear);

        prn("%s", ansi_indent_color[state->indentation]);
        if (el->list.type == LIST_PARENS)
        {
            prn(")");
        }
        else if (el->list.type == LIST_BRACKETS)
        {
            prn("]");
        }
        prn("%s", ansi_clear);

        state->line_len = 0;

        break;
    case AST_MAP:
        prn("%s", ansi_indent_color[state->indentation]);
        prn("{");
        prn("%s", ansi_clear);

        if (arrlen(el->map.kvs) > 4)
        {
            state->line_len = 0;
            state->indentation += 1;

            for (int i = 0; i < hmlen(el->map.kvs); i++)
            {

                state->dark = !state->dark;

                if (state->dark)
                {
                    prn("%s", ansi_dark_background);
                }
                else
                {
                    prn("%s", ansi_bright_background);
                }

                if (i > 0)
                    print_indent(state->indentation);

                prn("%s", ansi_clear);

                state->line_len = 0;
                _print_ast(state, &el->map.kvs[i].key);
                prn(" ");
                _print_ast(state, &el->map.kvs[i].value);

                if (i < hmlen(el->map.kvs) - 0)
                {
                    prn("\n");
                }
            }

            state->indentation -= 1;
        }
        else
        {
            state->line_len += hmlen(el->map.kvs);

            for (int i = 0; i < hmlen(el->map.kvs); i++)
            {
                _print_ast(state, &el->map.kvs[i].key);
                prn(" ");
                _print_ast(state, &el->map.kvs[i].value);

                if (i < hmlen(el->map.kvs) - 1)
                {
                    prn(",");
                    prn(" ");
                }
            }
        }

        if (state->dark)
        {
            prn("%s", ansi_dark_background);
        }
        else
        {
            prn("%s", ansi_bright_background);
        }

        // print_indent(state->indentation);

        prn("%s", ansi_clear);

        prn("%s", ansi_indent_color[state->indentation]);
        prn("}");
        prn("%s", ansi_clear);

        state->line_len = 0;

        break;
    case AST_EMPTY:
        break;
    default:
        prn("unhandled print: %d\n", el->ast_type);
        sai_assert(0);
    }

    if (el->value_type != NULL)
    {
        prn("%s", ansi_blue);
        _print_ast(state, el->value_type);
        prn("%s", ansi_clear);
    }
}

void prin_ast(AST *el)
{
    PrintASTState state = {};
    _print_ast(&state, el);
}

void print_ast(AST *el)
{
    prin_ast(el);
    prn("\n");
}

void print_ast2(AST el)
{
    print_ast(&el);
}

#define log_ast(el)                        \
    printf("%s:%d: ", __FILE__, __LINE__); \
    print_ast(el);

void print_ast_list(AST *els)
{
    for (AST *e = &els[0], *end = &arrlast(els) + 1; e != end; e++)
        print_ast(e);
}

AST *ast_in(AST *list, int n)
{
    sai_assert(list->ast_type == AST_LIST);
    sai_assert(n >= 0 && n < arrlen(list->list.elements));
    return &list->list.elements[n];
}

AST *ast_last(AST *list)
{
    sai_assert(list->ast_type == AST_LIST);
    return ast_in(list, arrlen(list->list.elements) - 1);
}

int ast_eq(AST *n1, AST *n2);

AST *get_in_map(AstKV *map, AST *k)
{
    for (int i = 0; i < hmlen(map); i++)
    {
        if (ast_eq(&map[i].key, k))
            return &map[i].value;
    }

    return NULL;
}

int is_submap_of(AstKV *sub, AstKV *map)
{
    if (hmlen(sub) > hmlen(map))
        return 0;

    for (int i = 0; i < hmlen(sub); i++)
    {
        AST *v = get_in_map(map, &sub[i].key);
        if (v == NULL || !ast_eq(&sub[i].value, v))
            return 0;
    }
    return 1;
}

int contains_all_keys(AstKV *map, AstKV *keys)
{
    if (hmlen(keys) > hmlen(map))
        return 0;

    for (int i = 0; i < hmlen(keys); i++)
    {
        if (get_in_map(map, &keys[i].key) == NULL)
            return 0;
    }

    return 1;
}

int ast_eq(AST *n1, AST *n2)
{
    sai_assert(n1 != NULL);
    sai_assert(n2 != NULL);
    if (n1->ast_type != n2->ast_type)
        return 0;

    switch (n1->ast_type)
    {
    case AST_LIST:
        if (arrlen(n1->list.elements) != arrlen(n2->list.elements))
            return 0;
        for (int i = 0; i < arrlen(n1->list.elements); i++)
        {
            if (!ast_eq(&n1->list.elements[i], &n2->list.elements[i]))
                return 0;
        }
        return 1;
        break;
    case AST_MAP:
        if (hmlen(n1->map.kvs) != hmlen(n2->map.kvs))
            return 0;
        for (int i = 0; i < hmlen(n1->map.kvs); i++)
        {
            AST *v = get_in_map(n2->map.kvs, &n1->map.kvs[i].key);
            if (v == NULL || !ast_eq(&n1->map.kvs[i].value, v))
                return 0;
        }
        return 1;
        break;
    case AST_SYMBOL:
        return strcmp(n1->symbol, n2->symbol) == 0;
        break;
    case AST_NUMBER:
        return n1->number == n2->number;
        break;
    case AST_BOOLEAN:
        return n1->boolean == n2->boolean;
        break;
    case AST_STRING:
        return strcmp(n1->string, n2->string) == 0;
        break;
    default:
        log("unhandled type -- ast: ");
        print_ast(n1);
        sai_assert(0);
        break;
    }
}

AST list0()
{
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = NULL}};
}

AST list1(AST n1)
{
    AST *els = NULL;
    arrpush(els, n1);
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST vector(AST l)
{
    sai_assert(l.ast_type == AST_LIST);
    l.list.type = LIST_BRACKETS;
    return l;
}

AST map1(AST k, AST v)
{
    AstKV *kvs = NULL;
    hmput(kvs, k, v);
    return (AST){.ast_type = AST_MAP, .map = {.kvs = kvs}};
}

AST list2(AST n1, AST n2)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list3(AST n1, AST n2, AST n3)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    arrpush(els, n3);
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list4(AST n1, AST n2, AST n3, AST n4)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    arrpush(els, n3);
    arrpush(els, n4);
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list5(AST n1, AST n2, AST n3, AST n4, AST n5)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    arrpush(els, n3);
    arrpush(els, n4);
    arrpush(els, n5);
    return (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST symbol(char *sym)
{
    return (AST){.ast_type = AST_SYMBOL, .symbol = sym};
}

AST with_type(AST node, AST *type)
{
    node.value_type = type;
    return node;
}

AST *new_symbol(char *sym)
{
    AST *s = (AST *)malloc(sizeof(AST));
    *s = symbol(sym);
    return s;
}

AST ast_string(char *str)
{
    return (AST){.ast_type = AST_STRING, .string = str};
}

AST *new_list(AST list)
{
    AST *l = (AST *)malloc(sizeof(AST));
    *l = list;
    return l;
}

AST *new_number(int n)
{
    AST *num = (AST *)malloc(sizeof(AST));
    num->number = n;
    return num;
}

AST *arr_to_list(AST *arr)
{
    AST *l = (AST *)malloc(sizeof(AST));
    l->ast_type = AST_LIST;
    l->list = (List){.type = LIST_PARENS, .elements = arr};
    return l;
}

AST *new_map(AST map)
{
    AST *m = (AST *)malloc(sizeof(AST));
    *m = map;
    return m;
}

AST parse(ParseState *state);

AST parse_list(ParseState *state, ListType list_type)
{
    int start = state->pos;
    int source_start = state->tokens[start].start;
    int source_stop = -1;
    state->pos++; // skip over initial opening token
    List list = {.type = list_type, .elements = NULL};

    TokenType closing_type = closing_types[list_type];

    while (1)
    {
        if (state->pos >= arrlen(state->tokens))
        {
            log("\033[0;31mno closing parens:\033[0m\n");
            sai_assert(0);
        }

        if (state->tokens[state->pos].type == closing_type)
        {
            source_stop = state->tokens[state->pos].stop;
            state->pos++;
            break;
        }

        AST ast = parse(state);
        if (ast.ast_type != AST_EMPTY)
        {
            arrpush(list.elements, ast);
        }
    }

    return (AST){.ast_type = AST_LIST, .list = list, .source = (SourceMapping){.start = source_start, .stop = source_stop}};
}

AST parse_map(ParseState *state)
{
    int start = state->pos;
    int source_start = state->tokens[start].start;
    int source_stop = -1;
    state->pos++; // skip over initial opening token
    Map map = {.kvs = NULL};

    int is_key = 1;

    AST key_ast = {};

    while (1)
    {
        if (state->pos >= arrlen(state->tokens))
        {
            log("\033[0;31mno closing braces:\033[0m\n");
            sai_assert(0);
        }

        if (state->tokens[state->pos].type == TOKEN_CLOSE_BRACES)
        {
            if (!is_key)
            {
                log("uneven number of kvs in map\n");
                sai_assert(0);
            }

            source_stop = state->tokens[state->pos].stop;
            state->pos++;
            break;
        }

        AST ast = parse(state);
        if (ast.ast_type != AST_EMPTY)
        {
            if (is_key)
            {
                key_ast = ast;
            }
            else
            {
                hmput(map.kvs, key_ast, ast);
            }
            is_key = !is_key;
        }
    }

    return (AST){.ast_type = AST_MAP, .map = map, .source = (SourceMapping){.start = source_start, .stop = source_stop}};
}

AST value_type_int = {.ast_type = AST_SYMBOL, .symbol = ":int"};
AST value_type_boolean = {.ast_type = AST_SYMBOL, .symbol = ":int"};
AST value_type_void = {.ast_type = AST_SYMBOL, .symbol = ":void"};
AST value_type_symbol = {.ast_type = AST_SYMBOL, .symbol = ":symbol"};

AST *value_type_string()
{
    AST *string = (AST *)malloc(sizeof(AST));
    *string = list1(symbol(":char"));
    string->list.type = LIST_BRACKETS;
    return string;
}

AST parse(ParseState *state)
{
    Token token = state->tokens[state->pos];

    if (token.type == TOKEN_OPEN_PARENS)
    {
        return parse_list(state, LIST_PARENS);
    }
    else if (token.type == TOKEN_OPEN_BRACKET)
    {
        return parse_list(state, LIST_BRACKETS);
    }
    else if (token.type == TOKEN_OPEN_BRACES)
    {
        return parse_map(state);
    }
    else if (token.type == TOKEN_SYMBOL)
    {
        state->pos++;
        int len = token.stop - token.start;
        char *sym = (char *)malloc(sizeof(char) * (len + 1));
        sym[len] = '\0';
        memcpy(sym, state->code + token.start, len);

        if (strcmp(sym, "true") == 0)
        {
            return (AST){.ast_type = AST_BOOLEAN, .boolean = 1, .value_type = &value_type_int};
        }
        else if (strcmp(sym, "false") == 0)
        {
            return (AST){.ast_type = AST_BOOLEAN, .boolean = 0, .value_type = &value_type_int};
        }
        else
        {
            return (AST){.ast_type = AST_SYMBOL, .symbol = sym};
        }
    }
    else if (token.type == TOKEN_STRING)
    {
        state->pos++;
        int len = token.stop - 1 - (token.start + 1);
        char *str = (char *)malloc(sizeof(char) * (len + 1));
        str[len] = '\0';
        memcpy(str, state->code + token.start + 1, len);
        return (AST){.ast_type = AST_STRING, .value_type = value_type_string(), .symbol = str};
    }
    else if (token.type == TOKEN_NUMBER)
    {
        state->pos++;
        int num = 0;
        int negative_number = state->code[token.start] == '-';
        int len = token.stop - token.start - negative_number;
        for (int i = 0; i < len; i++)
        {
            num += (state->code[token.stop - i - 1] - '0') * pow(10, i);
        }
        return (AST){.ast_type = AST_NUMBER, .value_type = &value_type_int, .number = negative_number ? -num : num};
    }
    else if (token.type == TOKEN_WHITESPACE || token.type == TOKEN_NEWLINE || token.type == TOKEN_COMMENT)
    {
        state->pos++;
        return (AST){.ast_type = AST_EMPTY};
    }
    else
    {
        log("\033[0;31munhandled token type: %s\033[0m\n", token_names[token.type]);
        sai_assert(0);
        // state.pos++;
    }
}

AST *parse_all(const char *code, Token *tokens)
{
    ParseState state = {code, tokens, 0};
    AST *root_nodes = NULL;

    while (state.pos < arrlen(tokens))
    {
        AST ast = parse(&state);
        if (ast.ast_type != AST_EMPTY)
        {
            arrpush(root_nodes, ast);
        }
    }

    return root_nodes;
}

////////////////// Add types ///////////////////////////

typedef struct TypeState
{
    TypeKV *globals;
} TypeState;

int in_types(TypeKV *types, AST *e)
{
    for (int i = 0; i < hmlen(types); i++)
    {
        if (ast_eq(types[i].key, e))
            return 1;
    }
    return 0;
}

AST *get_types(TypeKV *types, AST *e)
{
    for (int i = 0; i < hmlen(types); i++)
    {
        if (ast_eq(types[i].key, e))
            return types[i].value;
    }

    return NULL;
}

////////////////// Transform to C //////////////////////

typedef struct CTransformState
{
    int last_prepend_pos;
    int gensym;
} CTransformState;

typedef struct SymAST
{
    char *sym;
    AST ast;
} SymAST;

char *gensym(CTransformState *state)
{
    int len = snprintf(NULL, 0, "%d", state->gensym) + 6 + 1;
    char *str = malloc(len * sizeof(char));
    snprintf(str, len, "gensym%d", state->gensym);
    state->gensym++;
    return str;
}

SymAST transform(CTransformState *state, AST *node);

SymAST transform_if(CTransformState *state, AST *node)
{
    char *sym = gensym(state);
    AST do_block = list1(symbol("upscope"));

    sai_assert(node->value_type);
    int returns_void = ast_eq(node->value_type, &value_type_void);

    SymAST cond = transform(state, &node->list.elements[1]);
    if (cond.sym != NULL)
    {
        arrpush(do_block.list.elements,
                list2(symbol("var"), with_type(symbol(cond.sym), cond.ast.value_type)));
        arrpush(do_block.list.elements,
                cond.ast);
        node->list.elements[1] = symbol(cond.sym);
    }
    else
    {
        node->list.elements[1] = cond.ast;
    }

    AST *return_type = node->list.elements[2].value_type;

    {
        SymAST branch = transform(state, &node->list.elements[2]);
        if (branch
                .ast.value_type == NULL)
        {
            log("missing type: %d\n", branch.ast.ast_type);
            print_ast(&branch.ast);
            log("\n");
            sai_assert(NULL);
        }

        if (ast_eq(branch.ast.value_type, &value_type_void))
        {
            node->list.elements[2] = branch.ast;
        }
        else if (branch.sym != NULL)
        {
            node->list.elements[2] =
                list4(symbol("do"),
                      list2(symbol("var"), with_type(symbol(branch.sym), branch.ast.value_type)),
                      branch.ast,
                      list3(symbol("set"), symbol(sym), symbol(branch.sym)));
        }
        else
        {
            node->list.elements[2] = list3(symbol("set"), symbol(sym), branch.ast);
        }
    }

    if (arrlen(node->list.elements) > 3)
    {

        {
            SymAST branch = transform(state, &node->list.elements[3]);
            if (branch.ast.value_type == NULL)
            {
                log("missing type: %d\n", branch.ast.ast_type);
                print_ast(&branch.ast);
                log("\n");
                sai_assert(NULL);
            }

            if (ast_eq(branch.ast.value_type, &value_type_void))
            {
                node->list.elements[3] = branch.ast;
            }
            else if (branch.sym != NULL)
            {
                node->list.elements[3] =
                    list4(symbol("do"),
                          list2(symbol("var"), with_type(symbol(branch.sym), branch.ast.value_type)),
                          branch.ast,
                          list3(symbol("set"), symbol(sym), symbol(branch.sym)));
            }
            else
            {
                node->list.elements[3] = list3(symbol("set"), symbol(sym), branch.ast);
            }
        }
    }

    node->no_semicolon = 1;
    arrpush(do_block.list.elements, *node);

    do_block.no_semicolon = 1;
    sai_assert(return_type != NULL);
    do_block.value_type = return_type;

    return (SymAST){.sym = returns_void ? NULL : sym, .ast = do_block};
}

SymAST transform_do(CTransformState *state, AST *node)
{
    char *sym = gensym(state);

    int returns_void = ast_eq(node->value_type, &value_type_void);

    AST new_do = list1(node->list.elements[0]);
    new_do.value_type = node->value_type;

    for (int i = 1; i < arrlen(node->list.elements); i++)
    {
        SymAST arg = transform(state, &node->list.elements[i]);

        if (arg.sym == NULL)
        {
            if (i == arrlen(node->list.elements) - 1 && !returns_void)
                arrpush(new_do.list.elements, list3(symbol("set"), symbol(sym), arg.ast));
            else
                arrpush(new_do.list.elements, arg.ast);
        }
        else
        {
            arrpush(new_do.list.elements, list2(symbol("var"), with_type(symbol(arg.sym), arg.ast.value_type)));
            arrpush(new_do.list.elements, arg.ast);

            if (i == arrlen(node->list.elements) - 1 && !returns_void)
                arrpush(new_do.list.elements, list3(symbol("set"), symbol(sym), symbol(arg.sym)));
        }
    }

    new_do.no_semicolon = 1;

    return (SymAST){.sym = returns_void ? NULL : sym, .ast = new_do};
}

SymAST transform_while(CTransformState *state, AST *node)
{
    AST new_while = list2(
        node->list.elements[0],
        node->list.elements[1]);

    for (int i = 2; i < arrlen(node->list.elements); i++)
    {
        SymAST child = transform(state, &node->list.elements[i]);

        if (child.sym == NULL)
        {
            arrpush(new_while.list.elements, child.ast);
        }
        else
        {
            arrpush(new_while.list.elements, list2(symbol("var"), with_type(symbol(child.sym), child.ast.value_type)));
            arrpush(new_while.list.elements, child.ast);
            arrpush(new_while.list.elements, symbol(child.sym));
        }
    }

    new_while.no_semicolon = 1;

    return (SymAST){.sym = NULL, .ast = new_while};
}

SymAST transform_var(CTransformState *state, AST *node)
{
    if (arrlen(node->list.elements) > 2)
    {
        SymAST value = transform(state, &node->list.elements[2]);

        if (value.ast.value_type == NULL)
        {
            log("var missing type: %d\n", value.ast.ast_type);
            print_ast(&value.ast);
            log("\n");
            sai_assert(NULL);
        }

        if (value.sym == NULL)
        {
            node->list.elements[2] = value.ast;
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else
        {
            node->list.elements[2] = symbol(value.sym);
            AST upscope = list4(
                symbol("upscope"),
                list2(symbol("var"), with_type(symbol(value.sym), value.ast.value_type)),
                value.ast,
                *node);
            upscope.value_type = &value_type_void;
            upscope.no_semicolon = 1;
            return (SymAST){.sym = NULL, .ast = upscope};
        }
    }
    else
    {
        return (SymAST){.sym = NULL, .ast = *node};
    }
}

SymAST transform_set(CTransformState *state, AST *node)
{
    sai_assert(arrlen(node->list.elements) == 3);

    SymAST value = transform(state, &node->list.elements[2]);

    if (value.sym == NULL)
    {
        node->list.elements[2] = value.ast;
        return (SymAST){.sym = NULL, .ast = *node};
    }
    else
    {
        node->list.elements[2] = symbol(value.sym);
        AST upscope = list4(
            symbol("upscope"),
            list2(symbol("var"), with_type(symbol(value.sym), value.ast.value_type)),
            value.ast,
            *node);
        return (SymAST){.sym = NULL, .ast = upscope};
    }
}

SymAST transform_put(CTransformState *state, AST *node)
{
    sai_assert(arrlen(node->list.elements) == 4);

    SymAST value = transform(state, &node->list.elements[3]);

    if (value.sym == NULL)
    {
        node->list.elements[3] = value.ast;
        return (SymAST){.sym = NULL, .ast = *node};
    }
    else
    {
        node->list.elements[3] = symbol(value.sym);
        AST upscope = list4(
            symbol("upscope"),
            list3(symbol("var"), symbol(value.sym), symbol(":int")),
            value.ast,
            *node);
        return (SymAST){.sym = NULL, .ast = upscope};
    }
}

SymAST transform_insert(CTransformState *state, AST *node)
{
    sai_assert(arrlen(node->list.elements) == 4);

    SymAST value = transform(state, &node->list.elements[3]);

    if (value.sym == NULL)
    {
        node->list.elements[3] = value.ast;
        return (SymAST){.sym = NULL, .ast = *node};
    }
    else
    {
        AST *vt = node->list.elements[3].value_type;
        node->list.elements[3] = symbol(value.sym);
        node->list.elements[3].value_type = vt;
        AST upscope = list4(
            symbol("upscope"),
            list2(symbol("var"), node->list.elements[3]),
            value.ast,
            *node);
        upscope.value_type = &value_type_void;
        upscope.no_semicolon = 1;
        return (SymAST){.sym = NULL, .ast = upscope};
    }
}

SymAST transform_funcall(CTransformState *state, AST *node)
{
    char *sym = gensym(state);

    if (node->value_type == NULL)
    {
        log("missing value_type: %d, for\n", node->ast_type);
        print_ast(node);
        log("\n");
        sai_assert(NULL);
    }

    AST head = node->list.elements[0];

    int returns_void = ast_eq(node->value_type, &value_type_void);

    AST new_funcall = list1(head);
    new_funcall.value_type = node->value_type;
    AST upscope = list1(symbol("upscope"));
    upscope.no_semicolon = 1;

    for (int i = 1; i < arrlen(node->list.elements); i++)
    {
        SymAST arg = transform(state, &node->list.elements[i]);

        if (arg.sym == NULL)
        {
            arrpush(new_funcall.list.elements, arg.ast);
        }
        else
        {
            arrpush(upscope.list.elements,
                    list2(symbol("var"), with_type(symbol(arg.sym), arg.ast.value_type)));

            arrpush(upscope.list.elements, arg.ast);
            arrpush(new_funcall.list.elements, symbol(arg.sym));
        }
    }

    if (!returns_void)
    {
        arrpush(upscope.list.elements, list3(symbol("set"), symbol(sym), new_funcall));
    }
    else
    {
        arrpush(upscope.list.elements, new_funcall);
    }

    sai_assert(node->value_type != NULL);
    upscope.value_type = node->value_type;

    return (SymAST){.sym = returns_void ? NULL : sym, .ast = upscope};
}

SymAST transform_defn(CTransformState *state, AST *node)
{
    int is_void = ast_eq(ast_last(node->list.elements[1].value_type), &value_type_void);

    AST new_defn = list2(node->list.elements[0], transform(state, &node->list.elements[1]).ast);

    AST *args = node->list.elements[2].list.elements;
    AST *new_args = NULL;
    for (int i = 0; i < arrlen(args); i++)
    {
        AST sym = args[i];
        arrpush(new_args, list2(*sym.value_type, sym));
    }
    AST na = (AST){.ast_type = AST_LIST, .list = (List){.type = LIST_BRACKETS, .elements = new_args}};
    arrpush(new_defn.list.elements, na);

    for (int i = 2; i < arrlen(node->list.elements); i++)
    {
        SymAST res = transform(state, &node->list.elements[i]);

        if (res.sym == NULL)
        {
            arrpush(new_defn.list.elements, res.ast);
        }
        else
        {
            arrpush(new_defn.list.elements,
                    list2(symbol("var"), with_type(symbol(res.sym), node->list.elements[i].value_type)));

            arrpush(new_defn.list.elements, res.ast);
        }

        if (i == arrlen(node->list.elements) - 1 && is_void != 1)
        {
            if (res.sym == NULL)
            {
                arrpop(new_defn.list.elements);
                arrpush(new_defn.list.elements, list2(symbol("return"), res.ast));
            }
            else
            {
                arrpush(new_defn.list.elements, list2(symbol("return"), symbol(res.sym)));
            }
        }
    }

    new_defn.no_semicolon = 1;

    return (SymAST){.sym = new_defn.list.elements[1].symbol, .ast = new_defn};
}

SymAST transform_list(CTransformState *state, AST *node)
{
    switch (node->list.type)
    {
    case LIST_PARENS:
    {
        AST head = node->list.elements[0];
        sai_assert(head.ast_type == AST_SYMBOL);
        if (strcmp(head.symbol, "if") == 0)
        {
            return transform_if(state, node);
        }
        else if (strcmp(head.symbol, "do") == 0)
        {
            return transform_do(state, node);
        }
        else if (strcmp(head.symbol, "defmacro") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "var") == 0)
        {
            return transform_var(state, node);
        }
        else if (strcmp(head.symbol, "set") == 0)
        {
            return transform_set(state, node);
        }
        else if (strcmp(head.symbol, ":=") == 0)
        {
            return transform_insert(state, node);
        }
        else if (strcmp(head.symbol, "put") == 0)
        {
            return transform_put(state, node);
        }
        else if (strcmp(head.symbol, "+") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "zero?") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "=") == 0 || strcmp(head.symbol, "<=") == 0 || strcmp(head.symbol, "*") == 0 || strcmp(head.symbol, "!=") == 0 || strcmp(head.symbol, "+") == 0 || strcmp(head.symbol, "return") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "declare") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "declare-var") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "defstruct") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "defn") == 0)
        {
            return transform_defn(state, node);
        }
        else if (strcmp(head.symbol, "while") == 0)
        {
            return transform_while(state, node);
        }
        else
        {
            return transform_funcall(state, node);
        }
        break;
    }
    case LIST_BRACKETS:
    {
        return (SymAST){.sym = NULL, .ast = *node};
        break;
    }
    default:
        sai_assert(0);
        break;
    }
}

SymAST transform(CTransformState *state, AST *node)
{
    switch (node->ast_type)
    {
    case AST_LIST:
    {
        return transform_list(state, node);
        break;
    }
    case AST_SYMBOL:
    {
        char *currchr = node->symbol;
        while (*currchr++ != '\0')
        {
            if (*currchr == '-')
            {
                *currchr = '_';
            }
        }
        return (SymAST){.sym = NULL, .ast = *node};
        break;
    }
    default:
        return (SymAST){.sym = NULL, .ast = *node};
        break;
    }
}

AST *c_transform_all(AST *from)
{
    CTransformState state = {};
    AST *to = NULL;

    for (int i = 0; i < arrlen(from); i++)
    {
        SymAST node = transform(&state, &from[i]);
        arrpush(to, node.ast);
    }

    return to;
}

////////////////// Compile to C //////////////////////

typedef struct EnvEntry
{
    void *cvalue;
    AST *symbol;
    AST ast;
} EnvEntry;

typedef struct EnvKV
{
    char *key;
    EnvEntry value;
} EnvKV;

typedef struct CCompilationState
{
    String source;
    int indent;
    EnvKV *env;
} CCompilationState;

void c_compile(CCompilationState *state, AST node);

void c_compile_indentation(CCompilationState *state)
{
    for (int i = 0; i < state->indent; i++)
    {
        string(&state->source, " ");
    }
}

void type_to_string(String *str, AST type)
{
    switch (type.ast_type)
    {
    case AST_SYMBOL:
        if (type.symbol[0] == '?')
        {
            log("no concrete type: ");
            print_ast(&type);
            sai_assert(0);
        }
        strstr(str, type.symbol + 1);
        break;
    case AST_LIST:
    {
        int len = arrlen(&type.list.elements[0]);
        if (len == 1)
        {
            type_to_string(str, type.list.elements[0]);
            strstr(str, "*");
        }
        else
        {
            AST should_be_arrow = type.list.elements[len - 2];
            AST sym = symbol("->");
            sai_assert(ast_eq(&should_be_arrow, &sym));
            log_ast(&type);
            // have no nice way to compile function types without the function name :x
            sai_assert(0);
        }
        break;
    }
    default:
        log("unhandled type: ");
        print_ast(&type);
        sai_assert(0);
        break;
    }
}

void c_compile_type(CCompilationState *state, AST type)
{
    type_to_string(&state->source, type);
}

void c_compile_in_block(CCompilationState *state, AST node)
{
    state->indent += 2;
    c_compile_indentation(state);
    c_compile(state, node);

    if (node.no_semicolon == 0)
    {
        string(&state->source, ";");
    }
    string(&state->source, "\n");
    state->indent -= 2;
}

void c_compile_if(CCompilationState *state, AST node)
{
    string(&state->source, "if (");
    c_compile(state, node.list.elements[1]);
    string(&state->source, ") {\n");
    c_compile_in_block(state, node.list.elements[2]);
    c_compile_indentation(state);
    string(&state->source, "}");
    if (arrlen(node.list.elements) > 3)
    {
        string(&state->source, " else {\n");
        c_compile_in_block(state, node.list.elements[3]);
        c_compile_indentation(state);
        string(&state->source, "}");
    }
}

void c_compile_do(CCompilationState *state, AST node)
{
    string(&state->source, "{\n");
    for (int i = 1; i < arrlen(node.list.elements); i++)
    {
        c_compile_in_block(state, node.list.elements[i]);
    }
    c_compile_indentation(state);
    string(&state->source, "}");
}

void c_compile_cast(CCompilationState *state, AST node)
{
    string(&state->source, "(");
    c_compile_type(state, node.list.elements[1]);
    string(&state->source, ")");
    c_compile(state, node.list.elements[2]);
    // string(&state->source, ")");
}

void c_compile_get(CCompilationState *state, AST node)
{
    c_compile(state, node.list.elements[1]);
    string(&state->source, ".");
    c_compile(state, node.list.elements[2]);
    // string(&state->source, ")");
}

void c_compile_insert(CCompilationState *state, AST node)
{
    c_compile(state, node.list.elements[1]);
    string(&state->source, ".");
    c_compile(state, node.list.elements[2]);
    strstr(&state->source, " = ");
    c_compile(state, node.list.elements[3]);
}

void c_compile_defstruct(CCompilationState *state, AST node)
{
    string(&state->source, "typedef struct ");
    string(&state->source, node.list.elements[1].symbol);
    string(&state->source, " {\n");

    AstKV *map = node.list.elements[2].map.kvs;

    state->indent += 2;

    for (int i = 0; i < hmlen(map); i++)
    {
        c_compile_indentation(state);
        c_compile_type(state, map[i].value);
        string(&state->source, " ");
        string(&state->source, map[i].key.symbol);
        string(&state->source, ";\n");
    }

    state->indent -= 2;

    string(&state->source, "} ");
    string(&state->source, node.list.elements[1].symbol);
}

void c_compile_upscope(CCompilationState *state, AST node)
{
    for (int i = 1; i < arrlen(node.list.elements); i++)
    {
        // first element will already have indentation
        if (i > 1)
        {
            c_compile_indentation(state);
        }
        c_compile(state, node.list.elements[i]);

        if (node.list.elements[i].no_semicolon == 0)
        {
            string(&state->source, ";");
        }
        string(&state->source, "\n");
    }
}

void c_compile_while(CCompilationState *state, AST node)
{
    string(&state->source, "while (");
    c_compile(state, node.list.elements[1]);
    string(&state->source, ") {\n");
    for (int i = 2; i < arrlen(node.list.elements); i++)
    {
        c_compile_in_block(state, node.list.elements[i]);
    }
    c_compile_indentation(state);
    string(&state->source, "}");
}

void c_compile_defn(CCompilationState *state, AST node)
{
    AST *type = node.list.elements[1].value_type;
    AST return_type = *ast_last(type); // node.list.elements[3];
    // string(&state->source, type.symbol + 1);
    c_compile_type(state, return_type);

    char *funname = node.list.elements[1].symbol;

    strstr(&state->source, " ", funname, "(");

    AST args = node.list.elements[2];

    for (int i = 0; i < arrlen(args.list.elements); i++)
    {
        AST tuple = args.list.elements[i];
        AST type = tuple.list.elements[0];
        AST symbol = tuple.list.elements[1];

        c_compile_type(state, type);

        strstr(&state->source, " ", symbol.symbol);
        if (i != arrlen(args.list.elements) - 1)
        {
            string(&state->source, ", ");
        }
    }

    string(&state->source, ") {\n");

    // TODO: atm something adds an extra [] to functions
    // probably should remove that
    for (int i = 4; i < arrlen(node.list.elements); i++)
    {
        c_compile_in_block(state, node.list.elements[i]);
    }
    string(&state->source, "}\n");

    shput(state->env, funname, ((EnvEntry){.cvalue = NULL, .ast = node}));
}

void c_compile_funcall(CCompilationState *state, AST node)
{
    // if the type of the node and the type of the return value of the function differs
    // casting is necessary
    if (!ast_eq(node.value_type, &arrlast(node.list.elements[0].value_type->list.elements)))
    {
        strstr(&state->source, "(");
        c_compile_type(state, *node.value_type);
        strstr(&state->source, ")");
    }
    strstr(&state->source, node.list.elements[0].symbol, "(");

    for (int i = 1; i < arrlen(node.list.elements); i++)
    {
        c_compile(state, node.list.elements[i]);

        if (i != arrlen(node.list.elements) - 1)
        {
            string(&state->source, ", ");
        }
    }

    string(&state->source, ")");
}

void c_compile_return(CCompilationState *state, AST node)
{
    string(&state->source, "return ");
    for (int i = 1; i < arrlen(node.list.elements); i++)
    {
        c_compile(state, node.list.elements[i]);

        if (i != arrlen(node.list.elements) - 1)
        {
            string(&state->source, ", ");
        }
    }
}

void c_compile_var(CCompilationState *state, AST node)
{
    sai_assert(arrlen(node.list.elements) >= 2);

    AST sym = node.list.elements[1];
    AST type = *sym.value_type;
    sai_assert(sym.ast_type == AST_SYMBOL);

    c_compile_type(state, type);

    strstr(&state->source, " ", sym.symbol);

    if (arrlen(node.list.elements) > 2)
    {
        string(&state->source, " = ");

        if (type.ast_type == AST_SYMBOL)
        {
            int len = strlen(type.symbol);
            if (type.symbol[len - 1] != '*' && node.list.elements[2].ast_type == AST_SYMBOL && strcmp(node.list.elements[2].symbol, "NULL") == 0)
            {
                string(&state->source, "{}");
            }
            else
            {
                c_compile(state, node.list.elements[2]);
            }
        }
        else
        {
            c_compile(state, node.list.elements[2]);
        }
    }
}

void c_compile_declare_var(CCompilationState *state, AST node)
{
    sai_assert(arrlen(node.list.elements) == 3);

    AST sym = node.list.elements[1];
    AST type = *sym.value_type;
    sai_assert(sym.ast_type == AST_SYMBOL);

    c_compile_type(state, type);

    strstr(&state->source, " ", sym.symbol);
}

void c_compile_set(CCompilationState *state, AST node)
{
    AST sym = node.list.elements[1];
    sai_assert(sym.ast_type == AST_SYMBOL);
    strstr(&state->source, sym.symbol, " = ");
    c_compile(state, node.list.elements[2]);
}

void c_compile_in(CCompilationState *state, AST node)
{
    AST sym = node.list.elements[1];
    sai_assert(sym.ast_type == AST_SYMBOL);
    strstr(&state->source, sym.symbol, "[");
    c_compile(state, node.list.elements[2]);
    strstr(&state->source, "]");
}

void c_compile_put(CCompilationState *state, AST node)
{
    AST sym = node.list.elements[1];
    // sai_assert(sym.ast_type == AST_SYMBOL);
    c_compile(state, sym);
    strstr(&state->source, "[");
    c_compile(state, node.list.elements[2]);
    strstr(&state->source, "] = ");
    c_compile(state, node.list.elements[3]);
}

void c_compile_infix(CCompilationState *state, AST left, char *f, AST right)
{
    string(&state->source, "(");
    c_compile(state, left);
    strstr(&state->source, " ", f, " ");
    c_compile(state, right);
    string(&state->source, ")");
}

void c_compile_list(CCompilationState *state, AST node)
{
    switch (node.list.type)
    {
    case LIST_PARENS:
    {
        AST head = node.list.elements[0];
        sai_assert(head.ast_type == AST_SYMBOL);

        if (strcmp(head.symbol, "if") == 0)
        {
            return c_compile_if(state, node);
        }
        else if (strcmp(head.symbol, "do") == 0)
        {
            return c_compile_do(state, node);
        }
        else if (strcmp(head.symbol, "cast") == 0)
        {
            return c_compile_cast(state, node);
        }
        else if (strcmp(head.symbol, "get") == 0)
        {
            return c_compile_get(state, node);
        }
        else if (strcmp(head.symbol, ":=") == 0)
        {
            return c_compile_insert(state, node);
        }
        else if (strcmp(head.symbol, "declare") == 0)
        {
            // nothing
        }
        else if (strcmp(head.symbol, "defmacro") == 0)
        {
            // nothing
        }
        else if (strcmp(head.symbol, "declare-var") == 0)
        {
            return c_compile_declare_var(state, node);
        }
        else if (strcmp(head.symbol, "defstruct") == 0)
        {
            return c_compile_defstruct(state, node);
        }
        else if (strcmp(head.symbol, "upscope") == 0)
        {
            return c_compile_upscope(state, node);
        }
        else if (strcmp(head.symbol, "in") == 0)
        {
            return c_compile_in(state, node);
        }
        else if (strcmp(head.symbol, "put") == 0)
        {
            return c_compile_put(state, node);
        }
        else if (strcmp(head.symbol, "while") == 0)
        {
            return c_compile_while(state, node);
        }
        else if (strcmp(head.symbol, "var") == 0)
        {
            return c_compile_var(state, node);
        }
        else if (strcmp(head.symbol, "set") == 0)
        {
            return c_compile_set(state, node);
        }
        else if (strcmp(head.symbol, "zero?") == 0)
        {
            return c_compile_infix(state, node.list.elements[1], "==", (AST){.ast_type = AST_NUMBER, .number = 0, .value_type = &value_type_int});
        }
        else if (strcmp(head.symbol, "<=") == 0 || strcmp(head.symbol, ">=") == 0 || strcmp(head.symbol, ">") == 0 || strcmp(head.symbol, "<") == 0 || strcmp(head.symbol, "!=") == 0 || strcmp(head.symbol, "==") == 0 || strcmp(head.symbol, "*") == 0 || strcmp(head.symbol, "/") == 0 || strcmp(head.symbol, "+") == 0 || strcmp(head.symbol, "-") == 0 || strcmp(head.symbol, "%") == 0 || strcmp(head.symbol, "&&") == 0 || strcmp(head.symbol, "||") == 0)
        {
            sai_assert(arrlen(node.list.elements) == 3);
            return c_compile_infix(state, node.list.elements[1], head.symbol, node.list.elements[2]);
        }
        else if (strcmp(head.symbol, "defn") == 0)
        {
            return c_compile_defn(state, node);
        }
        else if (strcmp(head.symbol, "return") == 0)
        {
            return c_compile_return(state, node);
        }
        else
        {
            return c_compile_funcall(state, node);
        }
        break;
    }
    default:
        log("unhandled list: ");
        print_ast(&node);
        sai_assert(0);
        break;
    }
}

void c_compile_map(CCompilationState *state, AST node)
{
    AstKV *map = node.map.kvs;

    string(&state->source, "(");
    c_compile_type(state, *node.value_type);
    string(&state->source, ")");

    strstr(&state->source, "{\n");
    state->indent += 2;
    for (int i = 0; i < hmlen(map); i++)
    {
        c_compile_indentation(state);
        strstr(&state->source, ".");
        c_compile(state, map[i].key);
        strstr(&state->source, " = ");
        c_compile(state, map[i].value);
        strstr(&state->source, ",\n");
    }
    state->indent -= 2;
    c_compile_indentation(state);
    strstr(&state->source, "}");
}

void c_compile(CCompilationState *state, AST node)
{
    switch (node.ast_type)
    {
    case AST_LIST:
    {
        return c_compile_list(state, node);
        break;
    }
    case AST_MAP:
    {
        return c_compile_map(state, node);
        break;
    }
    case AST_SYMBOL:
    {
        string(&state->source, node.symbol);
        break;
    }
    case AST_NUMBER:
    {
        int len = snprintf(NULL, 0, "%d", node.number) + 1;
        char *str = malloc(len * sizeof(char));
        snprintf(str, len, "%d", node.number);
        string(&state->source, str);
        break;
    }
    case AST_STRING:
    {
        string(&state->source, "\"");

        for (char *c = node.string; *c != '\0'; c++)
        {
            if (*c == '\n')
            {
                string(&state->source, "\\n\"\n\"");
            }
            else
            {
                add_char(&state->source, *c);
            }
        }

        string(&state->source, "\"");

        break;
    }
    default:
        log("unhandled: ");
        print_ast(&node);
        log("\n");
        sai_assert(0);
        break;
    }
}

CCompilationState *c_compile_all(AST *from)
{
    CCompilationState *state = malloc(sizeof(CCompilationState));

    for (int i = 0; i < arrlen(from); i++)
    {
        AST node = from[i];
        c_compile(state, node);
        if (!node.no_semicolon)
        {
            string(&state->source, ";");
        }
        string(&state->source, "\n");
    }

    return state;
}