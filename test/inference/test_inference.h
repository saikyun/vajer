#include "minunit.h"
#include "src/lisp.h"
#include "slurp.h"

MU_TEST(test_basic_inference) {
    AST *ast = gen_ast(slurp("test/inference/basic.lisp"));
    printf("\n");
    print_ast(ast);
}

MU_TEST_SUITE(test_suite_inference) {
    MU_RUN_TEST(test_basic_inference);
}