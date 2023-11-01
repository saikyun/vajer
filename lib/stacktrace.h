#ifndef EMSCRIPTEN

/*
Modified 2023 Jona Ekenberg

The MIT License (MIT)

Copyright (c) 2015 Alexandru

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mach-o/getsect.h>
#include <stdio.h>
#include <mach-o/dyld.h>
#include <string.h>

char *stacktrace_program_name;

#include <signal.h>

// atos -o ./build/main 0x00000001000bf438

uint64_t StaticBaseAddress(void)
{
    const struct segment_command_64 *command = getsegbyname("__TEXT");
    uint64_t addr = command->vmaddr;
    return addr;
}

intptr_t ImageSlide(void)
{
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0)
        return -1;
    for (uint32_t i = 0; i < _dyld_image_count(); i++)
    {
        if (strcmp(_dyld_get_image_name(i), path) == 0)
            return _dyld_get_image_vmaddr_slide(i);
    }
    return 0;
}

uint64_t DynamicBaseAddress(void)
{
    return StaticBaseAddress() + ImageSlide();
}

// atos -o ./build/main 0x00000001000bf28c
// #> init_tiles (in main) (tiles.h:175)

// static - 0x0000000102b8b2a0
// 0x2B8B2A0
// dyn - 0x0000000102b8b2a0
// 0xBF2A0

#include <sys/types.h>
#include <unistd.h>

void printStackTrace(int called_from_sigint)
{

    static const unsigned int outBufSize = 2048;
    char outBuf[outBufSize];

    static const int traceSize = 16;
    void *trace[traceSize];
    char **messages = (char **)NULL;
    int i, trace_size = 0;
    int len = 0;

    trace_size = backtrace(trace, traceSize);

    messages = backtrace_symbols(trace, trace_size);

    pid_t pid = getpid();
    /*
    printf("pid: %d\n", pid);

    printf("image slide: %lx\n", ImageSlide());

    printf("static addr: %llx\n", StaticBaseAddress());

    printf("dyn addr: %llx\n", DynamicBaseAddress());

    printf("MESSAGES:\n");
    */

    static const int buflen = 1024;
    char syscom[buflen];

    // TODO: can I get load adress somewhere else?
    snprintf(syscom, buflen, "sample %d 1 2>/dev/null | grep \"Load Address\"", pid);

    // TODO: ... cleanup
    FILE *f = popen(syscom, "r");
    char *p = 0;
    if (f != NULL)
    {
        char buffer[buflen];
        memset(buffer, 0, buflen * sizeof(char));
        while (fgets(buffer, sizeof(buffer), f) != NULL)
        {
            // len += printf("yeah: %s", buffer);
        }
        p = strrchr(buffer, ' ');
    }

    pclose(f);

    if (p && *(p + 1))
    {
        // printf("load adress only: %s\n", p + 1);
        {
            /* skip first stack frame (points here) */
            // len += snprintf(outBuf, outBufSize, "Execution path:\n");
            for (i = 1; i < trace_size; ++i)
            {
                // printf("%d: %s\n", i, messages[i]);

                static const int buflen = 1024;
                char buf[buflen];
                memset(buf, 0, buflen * sizeof(char));
                // snprintf(buf, buflen, "#%s\n", messages[i]);
                len += snprintf(outBuf + len, outBufSize, "%s", buf);
                if (NULL == trace[i])
                {
                    len += snprintf(outBuf + len, outBufSize, " NO TRACE \n");
                    continue; // no stack info here
                }

                char syscom[buflen];

#ifdef __APPLE__
                // printf("trace: %p\n", trace[i]);
                // printf("atos -o %.256s -l %.11s %p\n", stacktrace_program_name, p + 1, trace[i]);
                snprintf(syscom, buflen, "atos -o %.256s -l %.11s %p", stacktrace_program_name, p + 1, trace[i]); // last parameter is the name of this app
#else
                snprintf(syscom, buflen, "addr2line %p -e %s", trace[i], stacktrace_program_name); // last parameter is the name of this app
#endif
                FILE *f = popen(syscom, "r");
                if (f != NULL)
                {
                    char buffer[buflen];
                    memset(buffer, 0, buflen * sizeof(char));
                    while (fgets(buffer, sizeof(buffer), f) != NULL)
                    {
                        printf("%s", buffer);
                        // len += snprintf(outBuf + len, outBufSize, "%s", buffer);
                    }
                    pclose(f);
                }
            }
        }
    }

    /*
        for (int i = 0; i < trace_size; i++)
        {
            printf("%d: %s\n", i, messages[i]);
        }
        */

    //     /* skip first stack frame (points here) */
    //     len += snprintf(outBuf, outBufSize, "Execution path:\n");
    //     for (i = 1; i < trace_size; ++i)
    //     {
    //         static const int buflen = 1024;
    //         char buf[buflen];
    //         memset(buf, 0, buflen * sizeof(char));
    //         snprintf(buf, buflen, "#%s\n", messages[i]);
    //         len += snprintf(outBuf + len, outBufSize, "%s", buf);
    //         if (NULL == trace[i])
    //         {
    //             len += snprintf(outBuf + len, outBufSize, "\n");
    //             continue; // no stack info here
    //         }

    //         char syscom[buflen];

    // #ifdef __APPLE__
    //         printf("atos -o %.256s %p", stacktrace_program_name, trace[i]);
    //         snprintf(syscom, buflen, "atos -o %.256s %p", stacktrace_program_name, trace[i]); // last parameter is the name of this app
    // #else
    //         snprintf(syscom, buflen, "addr2line %p -e %s", trace[i], stacktrace_program_name); // last parameter is the name of this app
    // #endif
    //         FILE *f = popen(syscom, "r");
    //         if (f != NULL)
    //         {
    //             char buffer[buflen];
    //             memset(buffer, 0, buflen * sizeof(char));
    //             while (fgets(buffer, sizeof(buffer), f) != NULL)
    //             {
    //                 len += snprintf(outBuf + len, outBufSize, "%s", buffer);
    //             }
    //             pclose(f);
    //         }
    //     }

    fprintf(stderr, "%s", outBuf);
    fflush(stderr);
    fflush(stdout);
}
/*
void initSigHandler(pfsig_hup sigHandler)
{
    // register for sigsegv
    struct sigaction sa;
    sa.sa_handler = (__sighandler_t)sigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGSEGV, &sa, NULL);
}
*/

void set_signal_handler(sig_t handler)
{
    signal(SIGABRT, handler);
}

void signal_handler(int sig)
{
    printStackTrace(1);
    switch (sig)
    {
    case SIGABRT:
        fprintf(stderr, "SIGABRT\n");
        break;
    }
    // exit(1);
}

void init_sig_handler(char *exe_path)
{
    set_signal_handler(signal_handler);
    stacktrace_program_name = exe_path;
}

#define CRITICAL() printStackTrace(0)

#else

#define CRITICAL() printf("stack trace goes here\n")
void init_sig_handler(char *exe_path)
{
}

#endif