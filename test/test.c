#include <stacktrace.h>

#include "minunit.h"
#include "test_tcc.h"
#include "test_parser.h"
#include "inference/test_inference.h"
#include "test_sdl.h"

int main(int argc, char **argv)
{
#ifdef STACKTRACE_ON
    init_sig_handler(argv[0]);
#endif
    int do_test = 1;

    // MU_RUN_TEST(test_infer_malloc);

    if (do_test)
    {
        MU_RUN_SUITE(tcc_suite);
        MU_RUN_SUITE(lisp_suite);
        MU_RUN_SUITE(test_suite_inference);
        // MU_RUN_SUITE(test_sdl_suite);
    }

    MU_REPORT();

    return MU_EXIT_CODE;
}