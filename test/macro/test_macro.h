#include "minunit.h"
#include "src/lisp.h"
#include "slurp.h"

MU_TEST(test_macro_list)
{
    TypeKV *env = standard_environment();
    eval(&env, slurp("test/macro/list.lisp"));
}

MU_TEST_SUITE(test_macro)
{
    MU_RUN_TEST(test_macro_list);
}