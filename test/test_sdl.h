#include "src/lisp.h"
#include "sai_string.h"
#include "slurp.h"

MU_TEST(test_sdl_init)
{
    eval(slurp("lisp/sdl-test.lisp"));
}

MU_TEST_SUITE(test_sdl_suite)
{
    MU_RUN_TEST(test_sdl_init);
}