#include "src/lisp.h"
#include "sai_string.h"
#include "slurp.h"

MU_TEST(test_sdl_init)
{
    TypeKV *env = standard_environment();
    eval(&env, slurp("lisp/sdl-test-structs.lisp"));
}

MU_TEST(test_spelsylt)
{
    TypeKV *env = standard_environment();
    eval(&env, slurp("lisp/spelsylt.lisp"));
}

MU_TEST_SUITE(test_sdl_suite)
{
    MU_RUN_TEST(test_sdl_init);
    MU_RUN_TEST(test_spelsylt);
}