#include "src/lisp.h"

MU_TEST(test_double_eval)
{
  VajerEnv *env = standard_environment();
  eval(env, "(defn inc [x] (+ x 1))");
  log("after first eval\n");
  print_env(env);
  eval(env, "(defn main [] (printf \"(inc 5) #=> %d\n\" (inc 5)))");
  //    eval(env, "(printf \"(inc 5) #=> %d\n\" (inc 5))");
}

MU_TEST(test_eval_macros)
{
  VajerEnv *env = standard_environment();
  eval(env, "(defn inc [x] (+ x 1))");
  eval(env, "(defmacro list"
            "  [args]"
            "  (var l NULL)"
            "  (var i 0)"
            "  (while (< i (arrlen args))"
            "    (do"
            "      (arrpush l (in args i))"
            "      (set i (inc i))))"
            "  (arr_to_list l))");
  eval(env, "(defn main"
            "  []"
            "  (printf \" adder : % d\n \" (list + 1 2))"
            "  (assert (== 3 (list + 1 2)))"
            "  (assert (== (list + 3 4) (list + 3 4)))"
            "  0)");
  //    eval(env, "(printf \"(inc 5) #=> %d\n\" (inc 5))");
}

/*

(defmacro other_list
  [args]
  (var l NULL)
  (var i 0)
  (while (< i (arrlen args))
    (do
      (arrpush l (in args i))
      (set i (inc i))))
  (arr_to_list l))

(defn main
  []
  (printf "adder: %d\n" (list + 1 2))
  (assert (== 3 (list + 1 2)))
  (assert (== (list + 3 4) (list + 3 4)))
  (assert (== 7 (other_list + 3 4)))
  0)


*/

MU_TEST_SUITE(test_eval)
{
  MU_RUN_TEST(test_double_eval);
  MU_RUN_TEST(test_eval_macros);
}