#include "lisp.h"
#include "infer_types.h"

/////////////////// Env ///////////////////////////

void add_op(EnvKV **env, char *type, char *op)
{
    AST *type_ast = (AST *)malloc(sizeof(AST));
    *type_ast = list4(symbol(type), symbol(type), symbol("->"), symbol(type));
    hmput(*env, new_symbol(op), type_ast);
}

EnvKV *standard_environment()
{
    EnvKV *env = NULL;

    add_op(&env, ":int", "+");
    add_op(&env, ":int", "<");
    add_op(&env, ":int", ">");
    add_op(&env, ":int", "<=");
    add_op(&env, ":int", ">=");
    add_op(&env, ":int", "%");
    add_op(&env, ":int", "!=");
    add_op(&env, ":int", "||");
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), symbol(":int"));
        hmput(env, new_symbol("=="), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(env, new_symbol("&&"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(env, new_symbol("-"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(env, new_symbol("*"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(list1(symbol("?T")), value_type_int, symbol("->"), symbol("?T"));
        hmput(env, new_symbol("in"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list5(list1(symbol("?T")), value_type_int, symbol("?T"), symbol("->"), value_type_void);
        hmput(env, new_symbol("put"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(env, new_symbol("set"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list5(symbol("?T"), symbol("?T2"), symbol("?T3"), symbol("->"), value_type_void);
        hmput(env, new_symbol(":="), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(env, new_symbol("var"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T2"), symbol("->"), value_type_void);
        hmput(env, new_symbol("while"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(env, new_symbol("declare"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(env, new_symbol("defstruct"), type);
    }

    return env;
}

////////////////// AST //////////////////////
//
//
//

typedef struct ScopeKV
{
    char *key;
    char *value;
} ScopeKV;

typedef struct Scope
{
    ScopeKV *kvs;
    struct Scope *children;
    struct Scope *parent;
} Scope;

char *get_in_scopes(Scope *scope, char *k)
{
    char *res = shget(scope->kvs, k);
    if (res)
        return res;
    if (scope->parent)
        return get_in_scopes(scope->parent, k);
    return NULL;
}

typedef struct ScopeState
{
    Scope *scope;
    int gensym;
} ScopeState;

char *scope_gensym(ScopeState *state, char *sym)
{
    int len = snprintf(NULL, 0, "%d", state->gensym) + strlen(sym) + 10;
    char *str = malloc(len * sizeof(char));
    snprintf(str, len, "%s__SCOPE__%d", sym, state->gensym);
    state->gensym++;
    return str;
}

void _ast_add_scope(ScopeState *state, AST *ast)
{
    switch (ast->ast_type)
    {
    case AST_LIST:
    {
        AST *elems = ast->list.elements;

        Scope *parent = 0;

        if (ast->list.type == LIST_PARENS && arrlen(elems) > 0)
        {
            if (elems[0].ast_type == AST_SYMBOL && strcmp(elems[0].symbol, "defn") == 0)
            {
                parent = state->scope;
                Scope *child = &(Scope){.parent = parent};
                state->scope = child;

                AST *args = elems[2].list.elements;

                for (int i = 0; i < arrlen(args); i++)
                {
                    shput(state->scope->kvs, args[i].symbol, scope_gensym(state, args[i].symbol));
                }
            }
        }

        for (int i = 0; i < arrlen(elems); i++)
        {
            _ast_add_scope(state, &elems[i]);
        }

        if (parent)
        {
            state->scope = parent;
        }

        break;
    }
    case AST_SYMBOL:
    {
        char *alias = get_in_scopes(state->scope, ast->symbol);
        if (alias)
        {
            ast->symbol = alias;
        }
        break;
    }

    default:
        break;
    }
}

void ast_add_scopes(AST *ast)
{
    ScopeState state = {.scope = &(Scope){}};
    for (int i = 0; i < arrlen(ast); i++)
    {
        _ast_add_scope(&state, &ast[i]);
    }
    // return state.types;
}

AST *vajer_ast(char *code)
{
    log("\e[34m>> code\n\e[0m%s\n", code);

    Token *tokens = tokenize(code, strlen(code));

    AST *root_nodes = parse_all(code, tokens);
    log("\n\e[35m>> parsed\n\e[0m");
    print_ast_list(root_nodes);
    log("\n");

    ast_add_scopes(root_nodes);

    log("\n\e[43m>> with scopes\e[0m\n");
    print_ast_list(root_nodes);
    log("\n");

    EnvKV *env = standard_environment();
    // AST *typed_nodes = add_type_all(&env, root_nodes);
    ast_resolve_types_all(env, code, root_nodes);
    log("\n\e[33m>> with types\n\e[0m");
    print_ast_list(root_nodes);
    log("\n");

    return root_nodes;
}

//
//
//
//
//

AST *c_ast(AST *ast)
{
    int do_print = 1;

    if (do_print)
    {
        log("\n\n\n\n\n\n>> \e[35mAST\e[0m\n");

        for (int i = 0; i < arrlen(ast); i++)
        {
            print_ast(&ast[i]);
        }
        log(">> \e[35mEnd of AST\e[0m\n");
    }

    AST *transformed_nodes = c_transform_all(ast);
    if (do_print)
    {
        log("\n");
        for (int i = 0; i < arrlen(transformed_nodes); i++)
        {
            print_ast(&transformed_nodes[i]);
        }
        log("\n");
    }

    return transformed_nodes;
}

////////////////// Eval / compile //////////////////////

void compile_to_file(char *code, char *path)
{
    int do_print = 0;

    char *source = c_compile_all(c_ast(vajer_ast(code)));

    FILE *f = fopen(path, "w");
    sai_assert(f != NULL);

    fputs("#include <stdio.h>\n"
          "#include <string.h>\n"
          "#include <SDL2/SDL.h>\n"
          "#include <assert.h>\n"
          "#include <time.h>\n"
          "#include <stdlib.h>\n"
          "#include <alloca.h>\n",
          f);
    fputs(source, f);
    fclose(f);
}

void eval(char *code)
{
    compile_to_file(code, "build/evaled.c");

    TCCState *s = tcc_new();
    // tcc_set_options(s, "-bt10");
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    sai_assert(tcc_add_file(s, "build/evaled.c") == 0);

    sai_assert(tcc_run(s, 0, NULL) == 0);

    /*
    char *source = c_compile_all(c_ast(vajer_ast(code)));

    int do_print = 0;

    if (do_print)
    {
        printf("source:\n%s\n", source);
    }

    TCCState *s = tcc_new();
    tcc_set_options(s, "-bt10");
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    String src = (String){};

    strstr(&src,
           "#include <stdio.h>\n"
           "#include <string.h>\n",
           "#include <SDL2/SDL.h>\n",
           "#include <assert.h>\n",
           "#include <time.h>\n",
           "#include <stdlib.h>\n",
           "#include <alloca.h>\n",
           source);

    sai_assert(tcc_compile_string(s, src.str) != -1);

    tcc_run(s, 0, NULL);
    */
}
