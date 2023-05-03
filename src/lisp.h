#include <ctype.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "sai_string.h"

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

const char *symbol_chars = "<=*-?+:";

int is_symbol(char c)
{
    return isalpha(c) || strchr(symbol_chars, c) != NULL;
}

Token token_symbol(TokenizeState *state)
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
            arrpush(tokens, token_string(&state));
        }
        else if (c == '\n')
        {
            arrpush(tokens, newline(&state));
        }
        else if (is_symbol(c))
        {
            arrpush(tokens, token_symbol(&state));
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
    int no_semicolon;
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
    default:
        printf("unhandled print: %d\n", el->type);
        assert(0);
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
            printf("\033[0;31mno closing parens:\033[0m\n");
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
        memcpy(sym, state->code + token.start, len);
        return (AST){.type = AST_SYMBOL, .symbol = sym};
    }
    else if (token.type == TOKEN_STRING)
    {
        state->pos++;
        int len = token.stop - 1 - (token.start + 1);
        char *str = (char *)malloc(sizeof(char) * (len + 1));
        str[len] = '\0';
        memcpy(str, state->code + token.start + 1, len);
        return (AST){.type = AST_STRING, .symbol = str};
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

AST list1(AST n1)
{
    AST *els = NULL;
    arrpush(els, n1);
    return (AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list2(AST n1, AST n2)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    return (AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list3(AST n1, AST n2, AST n3)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    arrpush(els, n3);
    return (AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST list4(AST n1, AST n2, AST n3, AST n4)
{
    AST *els = NULL;
    arrpush(els, n1);
    arrpush(els, n2);
    arrpush(els, n3);
    arrpush(els, n4);
    return (AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};
}

AST symbol(char *sym)
{
    return (AST){.type = AST_SYMBOL, .symbol = sym};
}

SymAST transform(CTransformState *state, AST *node);

SymAST transform_if(CTransformState *state, AST *node)
{
    char *sym = gensym(state);
    AST do_block = list2(symbol("do"),
                         list2(symbol("var"), symbol(sym)));

    SymAST cond = transform(state, &node->list.elements[1]);
    if (cond.sym != NULL)
    {
        arrpush(do_block.list.elements,
                list4(symbol("do"),
                      list2(symbol("var"), symbol(cond.sym)),
                      cond.ast,
                      list3(symbol("set"), symbol(sym), symbol(cond.sym))));
        node->list.elements[1] = symbol(cond.sym);
    }

    SymAST branch1 = transform(state, &node->list.elements[2]);
    if (branch1.sym != NULL)
    {
        node->list.elements[2] =
            list4(symbol("do"),
                  list2(symbol("var"), symbol(branch1.sym)),
                  branch1.ast,
                  list3(symbol("set"), symbol(sym), symbol(branch1.sym)));
    }
    else
    {
        node->list.elements[2] = list3(symbol("set"), symbol(sym), branch1.ast);
    }

    SymAST branch2 = transform(state, &node->list.elements[3]);
    if (branch2.sym != NULL)
    {
        node->list.elements[3] =
            list4(symbol("do"),
                  list2(symbol("var"), symbol(branch2.sym)),
                  branch1.ast,
                  list3(symbol("set"), symbol(sym), symbol(branch2.sym)));
    }
    else
    {
        node->list.elements[3] = list3(symbol("set"), symbol(sym), branch2.ast);
    }

    node->no_semicolon = 1;
    arrpush(do_block.list.elements, *node);

    char *ret = gensym(state);

    arrpush(do_block.list.elements, list3(symbol("set"), symbol(ret), symbol(sym)));

    do_block.no_semicolon = 1;

    return (SymAST){.sym = ret, .ast = do_block};
}

SymAST transform_do(CTransformState *state, AST *node)
{
    char *sym = gensym(state);

    AST last = arrlast(node->list.elements);

    SymAST res = transform(state, &last);

    if (res.sym == NULL)
    {
        arrpop(node->list.elements);
        arrpush(node->list.elements, list3(symbol("set"), symbol(sym), res.ast));
    }
    else
    {
        arrpush(node->list.elements, list3(symbol("set"), symbol(sym), symbol(res.sym)));
    }

    node->no_semicolon = 1;

    return (SymAST){.sym = sym, .ast = *node};
}

SymAST transform_defn(CTransformState *state, AST *node)
{
    char *ressym = NULL;

    int is_void = strcmp(node->list.elements[3].symbol + 1, "void") == 0;

    for (int i = 0; i < arrlen(node->list.elements); i++)
    {
        SymAST res = transform(state, &node->list.elements[i]);
        node->list.elements[i] = res.ast;

        if (i == arrlen(node->list.elements) - 1 && is_void != 1)
        {
            ressym = res.sym;
            if (res.sym == NULL)
            {
                arrpop(node->list.elements);
                arrpush(node->list.elements, list2(symbol("return"), res.ast));
            }
            else
            {
                arrpush(node->list.elements, list2(symbol("return"), symbol(res.sym)));
            }

            // adding to the list causes loop to keep going, so we break
            break;
        }
    }

    if (ressym)
    {
        arrins(node->list.elements, 4, list2(symbol("var"), symbol(ressym)));
    }

    AST *args = node->list.elements[2].list.elements;
    AST *new_args = NULL;
    for (int i = 0; i < arrlen(args); i += 2)
    {
        AST sym = args[i];
        AST type = args[i + 1];
        arrpush(new_args, list2(type, sym));
    }
    AST na = (AST){.type = AST_LIST, .list = (List){.type = LIST_BRACKETS, .elements = new_args}};
    node->list.elements[2] = na;

    return (SymAST){.sym = node->list.elements[1].symbol, .ast = *node};
}

SymAST transform_list(CTransformState *state, AST *node)
{
    switch (node->list.type)
    {
    case LIST_PARENS:
    {
        AST head = node->list.elements[0];
        assert(head.type == AST_SYMBOL);
        if (strcmp(head.symbol, "if") == 0)
        {
            return transform_if(state, node);
        }
        else if (strcmp(head.symbol, "do") == 0)
        {
            return transform_do(state, node);
        }
        else if (strcmp(head.symbol, "+") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "zero?") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "=") == 0 || strcmp(head.symbol, "<=") == 0 || strcmp(head.symbol, "*") == 0 || strcmp(head.symbol, "+") == 0 || strcmp(head.symbol, "return") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else if (strcmp(head.symbol, "defn") == 0)
        {
            return transform_defn(state, node);
        }
        else
        {
            printf("unhandled symbol, assuming funcall: %s\n", head.symbol);
            // assert(0);
            return (SymAST){.sym = NULL, .ast = *node};
        }
        break;
    }
    case LIST_BRACKETS:
    {
        return (SymAST){.sym = NULL, .ast = *node};
        break;
    }
    default:
        assert(0);
        break;
    }
}

SymAST transform(CTransformState *state, AST *node)
{
    switch (node->type)
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

typedef struct CCompilationState
{
    String source;
    int indent;
} CCompilationState;

void c_compile(CCompilationState *state, AST node);

void c_compile_indentation(CCompilationState *state)
{
    for (int i = 0; i < state->indent; i++)
    {
        string(&state->source, " ");
    }
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
    string(&state->source, "} else {\n");
    c_compile_in_block(state, node.list.elements[3]);
    c_compile_indentation(state);
    string(&state->source, "}");
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

void c_compile_defn(CCompilationState *state, AST node)
{
    AST type = node.list.elements[3];
    string(&state->source, type.symbol + 1);

    strstr(&state->source, " ", node.list.elements[1].symbol, "(");

    AST args = node.list.elements[2];

    for (int i = 0; i < arrlen(args.list.elements); i++)
    {
        AST tuple = args.list.elements[i];
        AST type = tuple.list.elements[0];
        AST symbol = tuple.list.elements[1];
        char *type_type = type.symbol + 1;
        if (strcmp(type_type, "string") == 0)
        {
            type_type = "char*";
        }
        strstr(&state->source, type_type, " ", symbol.symbol);
        if (i != arrlen(args.list.elements) - 1)
        {
            string(&state->source, ", ");
        }
    }

    string(&state->source, ") {\n");

    for (int i = 4; i < arrlen(node.list.elements); i++)
    {
        c_compile_in_block(state, node.list.elements[i]);
    }
    string(&state->source, "}\n");
}

void c_compile_funcall(CCompilationState *state, AST node)
{
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
    AST sym = node.list.elements[1];
    assert(sym.type == AST_SYMBOL);
    strstr(&state->source, "int ", sym.symbol);
}

void c_compile_set(CCompilationState *state, AST node)
{
    AST sym = node.list.elements[1];
    assert(sym.type == AST_SYMBOL);
    strstr(&state->source, sym.symbol, " = ");
    c_compile(state, node.list.elements[2]);
}

void c_compile_infix(CCompilationState *state, AST left, char *f, AST right)
{
    c_compile(state, left);
    strstr(&state->source, " ", f, " ");
    c_compile(state, right);
}

void c_compile_list(CCompilationState *state, AST node)
{
    switch (node.list.type)
    {
    case LIST_PARENS:
    {
        AST head = node.list.elements[0];
        assert(head.type == AST_SYMBOL);

        if (strcmp(head.symbol, "if") == 0)
        {
            return c_compile_if(state, node);
        }
        else if (strcmp(head.symbol, "do") == 0)
        {
            return c_compile_do(state, node);
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
            return c_compile_infix(state, node.list.elements[1], "==", (AST){.type = AST_NUMBER, .number = 0});
        }
        else if (strcmp(head.symbol, "<=") == 0 || strcmp(head.symbol, "*") == 0 || strcmp(head.symbol, "+") == 0 || strcmp(head.symbol, "-") == 0)
        {
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
            printf("unhandled head, assuming funcall: ");
            print_ast(&head);
            printf("\n");

            c_compile_funcall(state, node);
        }
        break;
    }
    default:
        printf("unhandled list: ");
        print_ast(&node);
        assert(0);
        break;
    }
}

void c_compile(CCompilationState *state, AST node)
{
    switch (node.type)
    {
    case AST_LIST:
    {
        return c_compile_list(state, node);
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
        strstr(&state->source, "\"", node.string, "\"");
        break;
    }
    default:
        printf("unhandled: ");
        print_ast(&node);
        printf("\n");
        assert(0);
        break;
    }
}

char *c_compile_all(AST *from)
{
    CCompilationState state = {};

    for (int i = 0; i < arrlen(from); i++)
    {
        AST node = from[i];
        c_compile(&state, node);
    }

    return state.source.str;
}