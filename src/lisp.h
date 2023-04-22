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

const char *symbol_chars = "<=*-?+";

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

    arrpush(do_block.list.elements, *node);

    char *ret = gensym(state);

    arrpush(do_block.list.elements, list3(symbol("set"), symbol(ret), symbol(sym)));

    return (SymAST){.sym = ret, .ast = do_block};
}

SymAST transform_do(CTransformState *state, AST *node)
{
    char *sym = gensym(state);

    AST last = arrlast(node->list.elements);

    SymAST res = transform(state, &last);

    AST *elements3 = NULL;
    arrpush(elements3, ((AST){.type = AST_SYMBOL, .symbol = "set"}));
    arrpush(elements3, ((AST){.type = AST_SYMBOL, .symbol = sym}));
    if (res.sym == NULL)
    {
        arrpop(node->list.elements);
        arrpush(elements3, res.ast);
    }
    else
    {
        arrpush(elements3, ((AST){.type = AST_SYMBOL, .symbol = res.sym}));
    }
    AST branch2 = ((AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = elements3}});

    arrpush(node->list.elements, branch2);

    return (SymAST){.sym = sym, .ast = *node};
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
        else if (strcmp(head.symbol, "=") == 0)
        {
            return (SymAST){.sym = NULL, .ast = *node};
        }
        else
        {
            printf("unhandled symbol: %s\n", head.symbol);
            assert(0);
        }
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
    default:
        // arrpush(state->to, *node);
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
        printf("sym: %s\n", node.sym);
    }

    return to;
}