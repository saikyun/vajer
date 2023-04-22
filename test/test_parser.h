#include "minunit.h"
#include "src/lisp.h"
#include "libtcc.h"

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
                       "    (* n (factorial (- n 1)))))";
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

    // "    (* n (factorial (- n 1)))))";
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
                       "    (* n (factorial (- n 1)))))";
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
    /*
    ```
    (if (zero? n) (do (print "hello") (+ 1 2 3)) 1)
    ```

    to

    ```
    [(var res)
     (if (zero? n)
       (do (print "hello")
           (def res2 (+ 1 2 3))
           (set res res2))
        (set res 1))
     # res somehow accessible, like getting an expr back from compile
     ]
    ```
    */

    const char *code = "(if (if (if false true 1337) (= 1 1) (zero? n)) (do (print \"hello\") (+ 1 2 3)) 1)";
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

    const char *code = "(if (zero? n) 0 1)";
    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);
    printf("\n");
    print_ast(root_nodes);
    printf("\n");

    AST *transformed_nodes = c_transform_all(root_nodes);
    printf("\n");
    for (int i = 0; i < arrlen(transformed_nodes); i++)
    {
        print_ast(&transformed_nodes[i]);
    }
    printf("\n");

    char *source = c_compile_all(transformed_nodes);
    printf("source:\n%s\n", source);
}


MU_TEST_SUITE(lisp_suite)
{
    // tokenize
    MU_RUN_TEST(test_tokenize_string);
    MU_RUN_TEST(test_tokenize_comment);
    MU_RUN_TEST(test_tokenize_defn);

    // parse
    MU_RUN_TEST(test_parse_defn);

    // transform
    MU_RUN_TEST(test_transform_if);
    MU_RUN_TEST(test_transform_if_do);

    // compile
    MU_RUN_TEST(test_compile_if);
}