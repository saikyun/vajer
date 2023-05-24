#include "minunit.h"
#include "src/lisp.h"
#include "slurp.h"

MU_TEST(test_basic_inference) {
    AST *ast = gen_ast(slurp("test/inference/basic.lisp"));
    printf("\n");
    print_ast(ast);
    AST x = ast[0].list.elements[1];
    print_ast(&x);
    AST sym = symbol(":int");
    mu_assert(x.value_type != NULL);
    mu_assert(ast_eq(x.value_type, &sym));
}

MU_TEST_SUITE(test_suite_inference) {
    MU_RUN_TEST(test_basic_inference);
}