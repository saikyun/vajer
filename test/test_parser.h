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

MU_TEST(test_eval_defn)
{
    const char *code = "(defn fac [n]\n"
                       "  (if (<= n 1)\n"
                       "    1\n"
                       "    (* n (fac (- n 1)))))";
    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);
    char *source = compile_all(root_nodes);

    printf("source:\n\n\"%s\"\n\n", source);

    TCCState *state = tcc_new();
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
    assert(tcc_compile_string(state, source) != -1);
    
    int size = tcc_relocate(state, NULL);
    assert(size != -1);
    void *mem = malloc(size);
    tcc_relocate(state, mem);
    
    int (*fac)(int) = tcc_get_symbol(state, "fac");
    assert(add != NULL);
    printf("fac(5): %d\n", fac(5));
    mu_assert(fac(5) == 120);
}

MU_TEST(test_eval_defn_with_more_stuff)
{
    const char *code = "(defn fac [n]\n"
                       "  (if (<= n 1)\n"
                       "    (if (= n 0) 0 1)\n"
                       "    (* n (fac (- n 1))))\n"
                       "  (if (<= n 1)\n"
                       "    1\n"
                       "    (* n (fac (- n 1)))))";
    Token *tokens = tokenize(code, strlen(code));
    AST *root_nodes = parse_all(code, tokens);
    char *source = compile_all(root_nodes);

    printf("source:\n\n\"%s\"\n\n", source);
}

MU_TEST_SUITE(lisp_suite)
{
    // tokenize
    MU_RUN_TEST(test_tokenize_string);
    MU_RUN_TEST(test_tokenize_comment);
    MU_RUN_TEST(test_tokenize_defn);

    // parse
    MU_RUN_TEST(test_parse_defn);

    // eval
    MU_RUN_TEST(test_eval_defn);
    MU_RUN_TEST(test_eval_defn_with_more_stuff);
}