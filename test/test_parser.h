#include "minunit.h"
#include "src/lisp.h"
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

    mu_assert(root_nodes[0].type == AST_LIST);
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

    const char *code = "(if (zero? n) 0 1)";
    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);
    /*
    printf("\n");
    print_ast(root_nodes);
    printf("\n");
    */

    AST *transformed_nodes = c_transform_all(root_nodes);
    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */
}

MU_TEST(test_transform_if_do)
{
    char *code = "(declare printf :void) (if (if (if false true 1337) (= 1 1) (zero? n)) (do (printf \"hello123\") (+ 1 2 3)) 1)";

    /*
    printf("\n");
    print_ast(root_nodes);
    printf("\n");
    */

    AST *transformed_nodes = gen_ast(code);

    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */
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
    AST *transformed_nodes = gen_ast(code);

    char *source = c_compile_all(transformed_nodes);
    // printf("source:\n%s\n", source);
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

    char *code = "(defn fac [n :int] :int\n"
                       "  (if (<= n 1)\n"
                       "    1\n"
                       "    (* n (fac (- n 1)))))";
    
    AST *transformed_nodes = gen_ast(code);

    char *source = c_compile_all(transformed_nodes);

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
        "(defn strlen-plus-n [str :string num :int] :int\n"
        "  (+ num (strlen str))\n"
        ")";
        
    AST *transformed_nodes = gen_ast(code);
    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */

    char *source = c_compile_all(transformed_nodes);
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
        "(defn plusser [nof :int] :int\n"
        "  (var res :int 0)\n"
        "  (while (> nof -1)\n"
        "    (set res (+ res nof))\n"
        "    (set nof (- nof 1))\n"
        "  )\n"
        "  res\n"
        ")";

    AST *transformed_nodes = gen_ast(code);

    /*
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");
    */

    char *source = c_compile_all(transformed_nodes);
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
    eval(code);
}

MU_TEST(test_add_type_declare)
{
    char *code = slurp("lisp/declare.lisp");
    AST *ast = gen_ast(code);

    /*
    printf("\ntype info:\n");
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
        printf(" type: %d\n", ast[i].value_type);
    }
    */

    // result of lule should have type void
    mu_assert(ast[1].list.elements[1].value_type != NULL);
    mu_assert(strcmp(ast[1].list.elements[1].value_type, ":void") == 0);

    //    printf("\ncode:\n%s\n", c_compile_all(ast));
}

MU_TEST(test_add_type_list)
{
    char *code = slurp("lisp/list.lisp");
    AST *ast = gen_ast(code);

    /*
    printf("\ntype info:\n");
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
        printf(" type: %s\n", ast[i].value_type);
    }

    // result of lule should have type void
    print_ast(&ast[1].list.elements[1].list.elements[2]);
    printf(" typerr: %s\n", ast[1].list.elements[1].list.elements[2].value_type);
    printf("\n");
    */

    mu_assert(ast[1].list.elements[1].list.elements[2].value_type != NULL);
    mu_assert(strcmp(ast[1].list.elements[1].list.elements[2].value_type, ":char") == 0);

    //    printf("\ncode:\n%s\n", c_compile_all(ast));
}

MU_TEST(test_add_type_intermediate)
{
    char *code = slurp("lisp/intermediate_types.lisp");
    AST *ast = gen_ast(code);

    /*
    printf("\ntype info:\n");
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
        printf(" type: %s\n", ast[i].value_type);
    }

    printf("\ncode:\n%s\n", c_compile_all(ast));
    printf("type: %s\n", ast[1].list.elements[1].list.elements[2].symbol);
    */

    mu_assert(strcmp(ast[1].list.elements[1].list.elements[2].symbol, ":void*") == 0);
}

MU_TEST(test_add_type_if)
{
    char *code = slurp("lisp/if.lisp");
    AST *ast = gen_ast(code);

    printf("\ntype info:\n");
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
        printf(" type: %s\n", ast[i].value_type);
    }

    printf("\ncode:\n%s\n", c_compile_all(ast));

    {
        AST *if_node = &ast[2].list.elements[5].list.elements[1];

        print_ast(if_node);
        printf(" type: %s\n", if_node->value_type);

        mu_assert(if_node->value_type != NULL);
        mu_assert(strcmp(if_node->value_type, ":void*") == 0);
    }

    {
        AST *if_node = &ast[2].list.elements[7].list.elements[1];

        printf("second\n");
        print_ast(if_node);
        printf(" type: %s\n", if_node->value_type);

        mu_assert(if_node->value_type != NULL);
        mu_assert(strcmp(if_node->value_type, ":void*") == 0);
    }

    {
        AST *if_node = &ast[2].list.elements[8].list.elements[2].list.elements[1];

        printf("last\n");
        print_ast(if_node);
        printf(" type: %s\n", if_node->value_type);

        mu_assert(if_node->value_type != NULL);
        mu_assert(strcmp(if_node->value_type, ":char*") == 0);
    }

    eval(code);
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