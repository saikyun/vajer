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
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), symbol(":int"));
        hmput(env, new_symbol(">"), type);
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

    return env;
}

////////////////// AST //////////////////////

AST *vajer_ast(char *code)
{
    int do_print = 0;

    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);

    EnvKV *env = standard_environment();

    AST *typed_nodes = add_type_all(&env, root_nodes);
    ast_resolve_types_all(env, typed_nodes);

    if (do_print)
    {
        printf("\n\nye {\n");
        for (int i = 0; i < arrlen(root_nodes); i++)
        {
            print_ast(&root_nodes[i]);
            printf("\n");
        }
        printf("} nye\n\n");
    }

    return typed_nodes;
}

AST *c_ast(AST *ast)
{
    int do_print = 0;

    if (do_print)
    {
        printf("\n\n\n\n\n\n>> \e[35mAST\e[0m\n");

        for (int i = 0; i < arrlen(ast); i++)
        {
            print_ast(&ast[i]);
        }
        printf(">> \e[35mEnd of AST\e[0m\n");
    }

    AST *transformed_nodes = c_transform_all(ast);
    if (do_print)
    {
        printf("\n");
        for (int i = 0; i < arrlen(transformed_nodes); i++)
        {
            print_ast(&transformed_nodes[i]);
        }
        printf("\n");
    }

    return transformed_nodes;
}

////////////////// Eval / compile //////////////////////

void eval(char *code)
{
    char *source = c_compile_all(c_ast(vajer_ast(code)));

    int do_print = 0;

    if (do_print)
    {
        printf("source:\n%s\n", source);
    }

    TCCState *s = tcc_new();
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

    assert(tcc_compile_string(s, src.str) != -1);

    tcc_run(s, 0, NULL);
}

void compile_to_file(char *code, char *path)
{
    int do_print = 0;

    char *source = c_compile_all(c_ast(vajer_ast(code)));

    FILE *f = fopen(path, "w");
    assert(f != NULL);

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