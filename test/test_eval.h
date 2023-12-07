#include "src/lisp.h"

MU_TEST(test_double_eval)
{
    VajerEnv *env = standard_environment();
    eval(env, "(defn inc [x] (+ x 1))");
    eval(env, "(defn main [] (printf \"(inc 5) #=> %d\n\" (inc 5)))");
    //    eval(env, "(printf \"(inc 5) #=> %d\n\" (inc 5))");
}

MU_TEST_SUITE(test_eval)
{
    MU_RUN_TEST(test_double_eval);
}