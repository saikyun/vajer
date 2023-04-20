#include <ctype.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef enum TokenType
{
    TOKEN_OPEN_PARENS,
    TOKEN_CLOSE_PARENS,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,

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
    while (state->pos < state->count && state->code[state->pos] == ' ')
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

const char *symbol_chars = "<=*-";

int is_symbol(char c)
{
    return isalpha(c) || strchr(symbol_chars, c) != NULL;
}

Token symbol(TokenizeState *state)
{
    int start = state->pos;
    while (state->pos < state->count && is_symbol(state->code[state->pos]))
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

Token string(TokenizeState *state)
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
        else if (c == ' ')
        {
            arrpush(tokens, whitespace(&state));
        }
        else if (c == '#')
        {
            arrpush(tokens, comment(&state));
        }
        else if (c == '"')
        {
            arrpush(tokens, string(&state));
        }
        else if (c == '\n')
        {
            arrpush(tokens, newline(&state));
        }
        else if (is_symbol(c))
        {
            arrpush(tokens, symbol(&state));
        }
        else if (isdigit(c))
        {
            arrpush(tokens, number(&state));
        }
        else
        {
            printf("\033[0;31munhandled: %c\033[0m\n", c);
            assert(0);
            // state.pos++;
        }
    }

    return tokens;
}

////////////////// Parsing //////////////////////

typedef enum ASTType
{
    AST_LIST,

    AST_STRING,
    AST_SYMBOL,
    AST_NUMBER,

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

typedef struct List
{
    ListType type;
    struct AST *elements;
} List;

typedef struct AST
{
    ASTType type;
    union
    {
        List list;
        char *string;
        char *symbol;
        int number;
    };
} AST;

typedef struct ParseState
{
    const char *code;
    Token *tokens;
    int pos;
} ParseState;

void print_ast(AST *el)
{
    switch (el->type)
    {
    case AST_SYMBOL:
        printf("%s", el->symbol);
        break;
    case AST_STRING:
        printf("\"%s\"", el->string);
        break;
    case AST_NUMBER:
        printf("%d", el->number);
        break;
    case AST_LIST:
        if (el->list.type == LIST_PARENS)
            printf("(");
        else if (el->list.type == LIST_BRACKETS)
            printf("[");

        for (int i = 0; i < arrlen(el->list.elements); i++)
        {
            print_ast(&el->list.elements[i]);

            if (i < arrlen(el->list.elements) - 1)
            {
                printf(" ");
            }
        }

        if (el->list.type == LIST_PARENS)
            printf(")");
        else if (el->list.type == LIST_BRACKETS)
            printf("]");

        break;
    case AST_EMPTY:
        break;
    }
}

AST parse(ParseState *state);

AST parse_list(ParseState *state, ListType list_type)
{
    int start = state->pos;
    state->pos++; // skip over initial opening token
    List list = {list_type, NULL};

    TokenType closing_type = closing_types[list_type];

    int done = 0;

    while (1)
    {
        if (state->pos >= arrlen(state->tokens))
        {
            printf("\033[0;31no closing parens:\033[0m\n");
            assert(0);
        }

        if (state->tokens[state->pos].type == closing_type)
        {
            state->pos++;
            break;
        }

        AST ast = parse(state);
        if (ast.type != AST_EMPTY)
        {
            arrpush(list.elements, ast);
        }
    }

    return (AST){AST_LIST, list};
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
    else if (token.type == TOKEN_SYMBOL)
    {
        state->pos++;
        int len = token.stop - token.start;
        char *sym = (char *)malloc(sizeof(char) * (len + 1));
        sym[len] = '\0';
        memcpy(sym, state->code + token.start * sizeof(char), len);
        return (AST){.type = AST_SYMBOL, .symbol = sym};
    }
    else if (token.type == TOKEN_NUMBER)
    {
        state->pos++;
        int num = 0;
        int len = token.stop - token.start;
        for (int i = 0; i < len; i++)
        {
            num += (state->code[token.stop - i - 1] - '0') * pow(10, i);
        }
        return (AST){.type = AST_NUMBER, .number = num};
    }
    else if (token.type == TOKEN_WHITESPACE || token.type == TOKEN_NEWLINE)
    {
        state->pos++;
        return (AST){.type = AST_EMPTY};
    }
    else
    {
        printf("\033[0;31munhandled token type: %s\033[0m\n", token_names[token.type]);
        assert(0);
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
        if (ast.type != AST_EMPTY)
        {
            arrpush(root_nodes, ast);
        }
    }

    return root_nodes;
}

////////////////// Compilation //////////////////////

typedef struct CompilationState
{
    char *source;
    int length;
    int capacity;
} CompilationState;

void append(CompilationState *state, char *src)
{
    int length = strlen(src);

    while (state->capacity <= state->length + length + 1)
    {
        int new_cap = (state->capacity + length + 1) * 2;
        void *res = realloc(state->source, new_cap * sizeof(char));
        assert(res != 0);
        state->source = res;
        state->capacity = new_cap;
    }

    memcpy(state->source + state->length, src, length);
    state->length += length;

    assert(state->length + 1 < state->capacity);

    memset(state->source + state->length + 1, '\0', 1);
}

int fac(int n)
{
    int res;
    if (n <= 1)
    {
        res = 1;
    }
    else
    {
        res = (n * (fac(n - 1)));
    }

    return res;
}

void compile(CompilationState *state, AST node);

void compile_defn(CompilationState *state, AST node)
{
    /*
      should generate something like:
      TYPE second.symbol(THIRD) {
          REST
      }
      */
    append(state, "int ");
    AST name = node.list.elements[1];
    assert(name.type == AST_SYMBOL);
    append(state, name.symbol);
    append(state, "(");

    AST args = node.list.elements[2];
    assert(args.type == AST_LIST && args.list.type == LIST_BRACKETS);
    for (int i = 0; i < arrlen(args.list.elements); i++)
    {
        AST arg = args.list.elements[i];
        assert(arg.type == AST_SYMBOL);
        append(state, "int ");
        append(state, arg.symbol);
        if (i < arrlen(args.list.elements) - 1)
            append(state, ", ");
    }
    append(state, ")");

    // body
    append(state, "{\n");

    for (int i = 3; i < arrlen(node.list.elements); i++)
    {
        compile(state, node.list.elements[i]);
    }

    append(state, "return res;\n");

    append(state, "}");
}

void compile_if(CompilationState *state, AST node)
{
    append(state, "int res;\n");
    append(state, "if ");
    compile(state, node.list.elements[1]);
    append(state, " {\n");
    append(state, "res = ");
    compile(state, node.list.elements[2]);
    append(state, ";\n");
    append(state, "} else {\n");
    for (int i = 3; i < arrlen(node.list.elements); i++)
    {
        if (i == arrlen(node.list.elements) - 1)
            append(state, "res = ");

        compile(state, node.list.elements[i]);

        if (i == arrlen(node.list.elements) - 1)
            append(state, ";");
    }
    append(state, "\n}\n\n");
}

void compile_default_call(CompilationState *state, AST node)
{
    AST f = node.list.elements[0];
    assert(f.type == AST_SYMBOL);

    append(state, f.symbol);
    append(state, "(");

    for (int i = 1; i < arrlen(node.list.elements); i++)
    {
        AST arg = node.list.elements[i];
        compile(state, arg);
        if (i < arrlen(node.list.elements) - 1)
            append(state, ", ");
    }

    append(state, ")");
}

void compile_infix(CompilationState *state, AST node)
{
    AST f = node.list.elements[0];
    assert(f.type == AST_SYMBOL);

    append(state, "(");
    compile(state, node.list.elements[1]);

    append(state, " ");

    append(state, f.symbol);

    append(state, " ");

    compile(state, node.list.elements[2]);
    append(state, ")");
}

void compile_call(CompilationState *state, AST node)
{
    AST head = node.list.elements[0];
    assert(head.type == AST_SYMBOL);
    if (strcmp(head.symbol, "defn") == 0)
    {
        compile_defn(state, node);
    }
    else if (strcmp(head.symbol, "if") == 0)
    {
        compile_if(state, node);
    }
    // TODO: maybe not so hard coded
    else if (strcmp(head.symbol, "<=") == 0 || strcmp(head.symbol, "*") == 0 || strcmp(head.symbol, "-") == 0)
    {
        compile_infix(state, node);
    }
    else
    {
        compile_default_call(state, node);
        /*
        printf("unhandled call: ");
        print_ast(&node);
        printf("\n\ncurrent source:\n\n%s\n\n", state->source);
        assert(0);
        */
    }
}

void compile_symbol(CompilationState *state, AST node)
{
    append(state, node.symbol);
}

void compile_number(CompilationState *state, AST node)
{
    int len = snprintf(NULL, 0, "%d", node.number);
    char *n = (char *)malloc(len + 1);
    snprintf(n, len + 1, "%d", node.number);
    append(state, n);
    // TODO: figure out why runtime error when freeing here
    // free(n);
}

void compile(CompilationState *state, AST node)
{
    switch (node.type)
    {
    case AST_LIST:
        if (node.type == LIST_PARENS)
        {
            compile_call(state, node);
        }
        break;
    case AST_SYMBOL:
        compile_symbol(state, node);
        break;
    case AST_NUMBER:
        compile_number(state, node);
        break;
    default:
        printf("unhandled compilation: ");
        print_ast(&node);
        printf("\n\ncurrent source:\n\n%s\n\n", state->source);
        assert(0);
        break;
    }
}

char *compile_all(AST *nodes)
{
    CompilationState state = {NULL, 0, 0};
    for (int i = 0; i < arrlen(nodes); i++)
    {
        compile(&state, nodes[i]);
    }
    return state.source;
}