#include "src/lisp.h"
#include "sai_string.h"

char *slurp(char *path)
{
    printf("loading file %s\n", path);

    FILE *f = fopen(path, "r");

    assert(f != NULL);

    size_t n = 0;

    fseek(f, 0, SEEK_END);
    n = ftell(f);
    printf("len: %zu\n", n);
    rewind(f);
    char *buffer = (char *)malloc((n + 1) * sizeof(char));
    assert(buffer != NULL);
    int read = fread(buffer, sizeof(char), n, f);
    printf("read: %d\n", read);

    buffer[n] = '\0';

    printf("contents: %s\n", buffer);

    fclose(f);

    return buffer;
}

MU_TEST(test_sdl_init)
{
    char *code = slurp("lisp/sdl-test.lisp");
    eval(code);
}

MU_TEST_SUITE(test_sdl_suite)
{
    MU_RUN_TEST(test_sdl_init);
}