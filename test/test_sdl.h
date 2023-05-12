#include "src/lisp.h"
#include "sai_string.h"
#include "slurp.h"

MU_TEST(test_sdl_init)
{
    char *code = slurp("lisp/sdl-test.lisp");
    eval(code);
}

MU_TEST_SUITE(test_sdl_suite)
{
    MU_RUN_TEST(test_sdl_init);
}