#include "minunit.h"
#include "src/eval.h"
#include "libtcc.h"
#include "slurp.h"

MU_TEST(test_tokenize_string)
{
    {
        const char *code = "\"hello\"";
        Token *tokens = tokenize(code, strlen(code));
        mu_assert(tokens[0].type == TOKEN_STRING);
    }

    {
        const char *code = "\"he\\\"ll\\\\\" \"wat\"";
        Token *tokens = tokenize(code, strlen(code));
        mu_assert(tokens[0].type == TOKEN_STRING);
        mu_assert(tokens[1].type == TOKEN_WHITESPACE);
        mu_assert(tokens[2].type == TOKEN_STRING);
        mu_assert(arrlen(tokens) == 3);
    }
}

MU_TEST(test_tokenize_comment)
{
    {
        const char *code = "aoeu # wat";
        Token *tokens = tokenize(code, strlen(code));
        mu_assert(tokens[0].type == TOKEN_SYMBOL);
        mu_assert(tokens[1].type == TOKEN_WHITESPACE);
        mu_assert(tokens[2].type == TOKEN_COMMENT);
    }
}

MU_TEST(test_tokenize_defn)
{
    const char *code = "(defn fac [n]\n"
                       "  (if (<= n 1)\n"
                       "    1\n"
                       "    (* n (fac (- n 1)))))";
    Token *tokens = tokenize(code, strlen(code));
    int i = 0;

    // "(defn fac [n]\n"
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_BRACKET);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_BRACKET);
    mu_assert(tokens[i++].type == TOKEN_NEWLINE);

    // "  (if (<= n 1)\n"
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_NUMBER);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
    mu_assert(tokens[i++].type == TOKEN_NEWLINE);

    // "    1\n"
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_NUMBER);
    mu_assert(tokens[i++].type == TOKEN_NEWLINE);

    // "    (* n (fac (- n 1)))))";
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_OPEN_PARENS);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_SYMBOL);
    mu_assert(tokens[i++].type == TOKEN_WHITESPACE);
    mu_assert(tokens[i++].type == TOKEN_NUMBER);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
    mu_assert(tokens[i++].type == TOKEN_CLOSE_PARENS);
}

MU_TEST(test_parse_defn)
{
    const char *code = "(defn fac [n]\n"
                       "  (if (<= n 1337)\n"
                       "    5\n"
                       "    (* n (fac (- n 1)))))";
    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);

    /*
        for (int i = 0; i < arrlen(root_nodes); i++)
        {
            print_ast(&root_nodes[i]);
        }
    */

    mu_assert(root_nodes[0].ast_type == AST_LIST);
}

MU_TEST(test_transform_if)
{
    /*
    ```
    [(if (zero? n) 0 1)]
    ```

    to

    ```
    [(var res)
     (if (zero? n) (set res 0) (set res 1))
     # res somehow accessible, like getting an expr back from compile
     ]
    ```
    */

    char *code = "(if (zero? n) 0 1)";

    /*
    printf("\n");
    print_ast(root_nodes);
    printf("\n");
    */

    EnvKV *env = standard_environment();
    c_ast(vajer_ast(&env, code));
}

MU_TEST(test_transform_if_do)
{
    char *code = "(if (if (if false true 1337) (= 1 1) (zero? n)) (do (printf \"hello123\" \"\") (+ 1 2)) 1)";

    /*
    printf("\n");
    print_ast(root_nodes);
    printf("\n");
    */

    EnvKV *env = standard_environment();
    c_ast(vajer_ast(&env, code));
}

MU_TEST(test_compile_if)
{
    /*
    ```
    (if (zero? n) 0 1)
    ```

    to

    ```
    (do (var gensym0)
        (if (zero? n)
            (set gensym0 0)
            (set gensym0 1))
        (set gensym1 gensym0))
    ```

    to

    ```
    int gensym0;
    if (n == 0) {
        gensym0 = 0;
    } else {
        gensym0 = 1;
    }
    gensym1 = gensym0;
    ```
    */

    char *code = "(if (zero? n) 0 1)";

    EnvKV *env = standard_environment();
    AST *transformed_nodes = c_ast(vajer_ast(&env, code));

    c_compile_all(transformed_nodes);
}

MU_TEST(test_compile_defn)
{
    /*
    (defn fac [n]
      (if (<= n 1337)
        5
        (* n (fac (- n 1)))))
    ```

    to

    ```
    (defn fac [n]
      (var gensym0)
      (if (<= n 1337)
        (set gensym0 5)
        (set gensym0 (* n (fac (- n 1)))))
      (return gensym0))
    ```

    to

    ```
    int fac(n) {
        int res;
        if (n <= 1337) {
            res = 5;
        } else {
            res = (n * fac(n - 1));
        }
        return res;
    }
    ```
    */

    char *code = "(defn fac [n]\n"
                 "  (if (<= n 1)\n"
                 "    1\n"
                 "    (* n (fac (- n 1)))))";

    EnvKV *env = standard_environment();
    AST *transformed_nodes = c_ast(vajer_ast(&env, code));

    char *source = c_compile_all(transformed_nodes)->source.str;

    // printf("source:\n%s\n", source);

    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    mu_assert(tcc_compile_string(s, source) != -1);

    int size = tcc_relocate(s, NULL);
    mu_assert(size != -1);
    void *mem = malloc(size);
    tcc_relocate(s, mem);

    int (*fac)(int) = tcc_get_symbol(s, "fac");
    mu_assert(fac != NULL);
    // printf("fac(5): %d\n", fac(5));
    mu_assert(fac(5) == 120);
}

MU_TEST(test_compile_two_types)
{
    char *code =
        "(declare strlen [[:char] -> :int])\n"
        "(defn strlen-plus-n [str num]\n"
        "  (+ num (strlen str))\n"
        ")";

    EnvKV *env = standard_environment();
    AST *transformed_nodes = c_ast(vajer_ast(&env, code));
    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */

    char *source = c_compile_all(transformed_nodes)->source.str;
    // printf("source:\n%s\n", source);

    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    String src = (String){};

    strstr(&src,
           "#include <stdio.h>\n"
           "#include <string.h>",
           source);

    // mu_assert(tcc_compile_string(s, source) != -1);
    mu_assert(tcc_compile_string(s, src.str) != -1);

    int size = tcc_relocate(s, NULL);
    mu_assert(size != -1);
    void *mem = malloc(size);
    tcc_relocate(s, mem);

    int (*strlen_plus_n)(char *, int) = tcc_get_symbol(s, "strlen_plus_n");
    mu_assert(strlen_plus_n != NULL);
    mu_assert(10 == strlen_plus_n("hello", 5));
}

MU_TEST(test_compile_while)
{
    char *code =
        "(defn plusser [nof]\n"
        "  (var res 0)\n"
        "  (while (> nof -1)\n"
        "    (do\n"
        "      (set res (+ res nof))\n"
        "      (set nof (- nof 1)))\n"
        "  )\n"
        "  res\n"
        ")";

    EnvKV *env = standard_environment();
    AST *transformed_nodes = c_ast(vajer_ast(&env, code));

    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */

    char *source = c_compile_all(transformed_nodes)->source.str;
    // printf("source:\n%s\n", source);

    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    String src = (String){};

    strstr(&src,
           "#include <stdio.h>\n"
           "#include <string.h>",
           source);

    mu_assert(tcc_compile_string(s, src.str) != -1);

    int size = tcc_relocate(s, NULL);
    mu_assert(size != -1);
    void *mem = malloc(size);
    tcc_relocate(s, mem);

    int (*plusser)(int) = tcc_get_symbol(s, "plusser");
    mu_assert(plusser != NULL);

    // don't ask me what this function does. I'm sorry.
    mu_assert(91 == plusser(13));
}

MU_TEST(test_compile_set_if)
{
    char *code = slurp("lisp/set-if.lisp");
    // char *code = slurp("lisp/funcall-if.lisp");

    EnvKV *env = standard_environment();
    eval(&env, code);
}

MU_TEST(test_add_type_declare)
{
    char *code = slurp("lisp/declare.lisp");
    EnvKV *env = standard_environment();
    AST *ast = c_ast(vajer_ast(&env, code));

    // result of lule should have type void
    mu_assert(ast[1].list.elements[1].value_type != NULL);
    mu_assert(ast_eq(ast[1].list.elements[1].value_type, &value_type_void));

    //    printf("\ncode:\n%s\n", c_compile_all(ast));
}

MU_TEST(test_add_type_list)
{
    char *code = slurp("lisp/list.lisp");
    EnvKV *env = standard_environment();
    AST *ast = c_ast(vajer_ast(&env, code));

    /*
    printf("\ntype info:\n");
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
    }

    print_ast(&ast[1].list.elements[1].list.elements[2]);
    */

    mu_assert(ast[1].list.elements[1].list.elements[2].value_type != NULL);
    AST sym = symbol(":char");

    mu_assert(ast_eq(ast[1].list.elements[1].list.elements[2].value_type, &sym));
}

MU_TEST(test_add_type_intermediate)
{
    char *code = slurp("lisp/intermediate_types.lisp");
    EnvKV *env = standard_environment();
    AST *ast = c_ast(vajer_ast(&env, code));

    /*
        printf("\ntype info:\n");
        for (int i = 0; i < arrlen(ast); i++)
        {
            print_ast(&ast[i]);
        }

        printf("\ncode:\n%s\n", c_compile_all(ast));
        print_ast(&ast[1].list.elements[2]);
    */

    AST sym = symbol(":void*");
    mu_assert(ast_eq(ast[1].list.elements[2].value_type, &sym));
}

MU_TEST(test_add_type_if)
{
    int do_print = 0;

    char *code = slurp("lisp/if.lisp");
    EnvKV *env = standard_environment();
    AST *ast = c_ast(vajer_ast(&env, code));

    if (do_print)
    {
        printf("\ntype info:\n");
        for (int i = 0; i < arrlen(ast); i++)
        {
            print_ast(&ast[i]);
        }

        printf("\ncode:\n%s\n", c_compile_all(ast)->source.str);
    }
    return;

    {
        AST *if_node = &ast[2].list.elements[5].list.elements[1];

        if (do_print)
        {
            print_ast(if_node);
        }

        mu_assert(if_node->value_type != NULL);
        AST sym = symbol(":void*");
        mu_assert(ast_eq(if_node->value_type, &sym));
    }

    {
        AST *if_node = &ast[2].list.elements[7].list.elements[1];

        if (do_print)
        {
            printf("second\n");
            print_ast(&ast[2]);
            print_ast(if_node);
        }

        mu_assert(if_node->value_type != NULL);
        AST sym = symbol(":void*");
        mu_assert(ast_eq(if_node->value_type, &sym));
    }

    {
        AST *if_node = &ast[2].list.elements[8].list.elements[2].list.elements[1];

        if (do_print)
        {
            printf("\n\n\n\nlast\n");
            print_ast(if_node);
        }

        mu_assert(if_node->value_type != NULL);
        AST sym = list1(symbol(":char"));

        mu_assert(ast_eq(if_node->value_type, &sym));
    }

    {
        EnvKV *env = standard_environment();
        compile_to_file(&env, code, "build/if.c");
    }

    {
        EnvKV *env = standard_environment();
        eval(&env, code);
    }
}

MU_TEST_SUITE(lisp_suite)
{
    // tokenize
    MU_RUN_TEST(test_tokenize_string);
    MU_RUN_TEST(test_tokenize_comment);
    MU_RUN_TEST(test_tokenize_defn);

    // parse
    MU_RUN_TEST(test_parse_defn);

    // add type
    MU_RUN_TEST(test_add_type_declare);
    MU_RUN_TEST(test_add_type_list);
    MU_RUN_TEST(test_add_type_intermediate);
    MU_RUN_TEST(test_add_type_if);

    // transform
    MU_RUN_TEST(test_transform_if);
    MU_RUN_TEST(test_transform_if_do);
    // compile
    MU_RUN_TEST(test_compile_if);
    MU_RUN_TEST(test_compile_defn);
    MU_RUN_TEST(test_compile_two_types);
    MU_RUN_TEST(test_compile_while);
    MU_RUN_TEST(test_compile_set_if);
}