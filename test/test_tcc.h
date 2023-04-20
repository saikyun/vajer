#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tinycc/libtcc.h"
#include "minunit.h"

MU_TEST(add) {
    TCCState *s = tcc_new();
    assert(s);
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    const char *program = "int add(int x, int y) {\n"
                          "  return x + y;\n"
                          "}";
    
    assert(tcc_compile_string(s, program) != -1);

    int size = tcc_relocate(s, NULL);
    assert(size != -1);
    void *mem = malloc(size);
    tcc_relocate(s, mem);
    
    int (*add)(int, int) = tcc_get_symbol(s, "add");
    assert(add != NULL);
    
    mu_check(1337 == add(10, 1327));
}

MU_TEST_SUITE(tcc_suite) {
    MU_RUN_TEST(add);
}