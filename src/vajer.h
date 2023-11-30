#include "lisp.h"
#include "eval.h"

typedef AST VajerExpr;

void vajer_pp(VajerExpr *expr)
{
    print_ast(expr);
}

EnvKV *cenv = NULL;

void vajer_eval_string(char *code)
{
    sai_assert(current_env != NULL);
    EnvKV *res_cenv = eval_ast(cenv, vajer_ast(&current_env, code));

    for (int i = 0; i < shlen(res_cenv); i++)
    {
        shput(cenv, res_cenv[i].key, res_cenv[i].value);
    }

    if (shgeti(cenv, "vajer$run") != -1)
    {
        EnvEntry entry = shget(cenv, "vajer$run");
        // print_ast(&entry.type);

        if (ast_eq(&arrlast(entry.type.list.elements), new_symbol(":int")))
        {
            int *(*run)() = entry.value;
            printf("%d\n", run());
        }
    }
}
