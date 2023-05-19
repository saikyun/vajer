#include "src/lisp.h"
#include "sai_string.h"
#include "slurp.h"

MU_TEST(test_sdl_init)
{
    char *code = slurp("lisp/sdl-test.lisp");
    // eval(code);
    compile_to_file(code, "build/sdl-test.c");

    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    tcc_add_file(s, "build/sdl-test.c");
    tcc_run(s, 0, NULL);
}

MU_TEST_SUITE(test_sdl_suite)
{
    MU_RUN_TEST(test_sdl_init);
}