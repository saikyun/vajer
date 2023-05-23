#include <stacktrace.h>

#include "minunit.h"
#include "test_tcc.h"
#include "test_parser.h"
#include "test_sdl.h"

int main(int argc, char **argv)
{
#ifdef STACKTRACE_ON
    init_sig_handler(argv[0]);
#endif
    //MU_RUN_SUITE(tcc_suite);
    MU_RUN_SUITE(lisp_suite);
    MU_RUN_SUITE(test_sdl_suite);

    MU_REPORT();
    
    return MU_EXIT_CODE;
}