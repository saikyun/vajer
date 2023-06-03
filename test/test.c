#include <stacktrace.h>
#include <stdio.h>

#include "minunit.h"
#include "test_tcc.h"
#include "test_parser.h"
#include "inference/test_inference.h"
#include "test_sdl.h"

int main(int argc, char **argv)
{
    log_stream = open_memstream(&log_buffer, &log_size);

    /*
        char *bp;
        size_t size;
        FILE *stream;

        stream = open_memstream(&bp, &size);
        fprintf(stream, "hello\n");
        fclose(stream);

        free(bp);
        bp = NULL;

        stream = open_memstream(&bp, &size);
        fprintf(stream, "123\n");
        fclose(stream);

        printf("%s", bp);
    */

#ifdef STACKTRACE_ON
#ifndef COMPILED_WITH_TCC
    init_sig_handler(argv[0]);
#endif
#endif
    int do_test = 1;

    //MU_RUN_TEST(test_infer_defn_and_call);
    //dump_log();

    if (do_test)
    {
        MU_RUN_SUITE(tcc_suite);
        MU_RUN_SUITE(lisp_suite);
        MU_RUN_SUITE(test_suite_inference);
        MU_RUN_SUITE(test_sdl_suite);
    }

    MU_REPORT();

    return MU_EXIT_CODE;
}