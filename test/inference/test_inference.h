#include "minunit.h"
#include "src/lisp.h"
#include "src/infer_types.h"
#include "slurp.h"

MU_TEST(test_basic_unification)
{
    // translated from https://github.com/eliben/paip-in-clojure/blob/master/src/paip/11_logic/unification_test.clj

    {
        AST x = symbol("x");
        EnvKV *env = NULL;
        int res = unify(&x, &x, &env);
        mu_assert(res);
    }
    {
        AST x = list2(symbol("?x"), symbol("?y"));
        EnvKV *env = NULL;
        int res = unify(&x, &x, &env);
        mu_assert(res);
    }
    {
        AST x = symbol("x");
        AST y = symbol("y");
        EnvKV *env = NULL;
        int res = unify(&x, &y, &env);
        mu_assert(res == 0);
    }
    {
        AST x = list3(symbol("+"), symbol("?x"), symbol("1"));
        AST y = list3(symbol("+"), symbol("2"), symbol("1"));
        EnvKV *env = NULL;
        int res = unify(&x, &y, &env);
        mu_assert(res);
        AST sym2 = symbol("2");
        mu_assert(ast_eq(get_types(env, new_symbol("?x")), &sym2));
    }
    {
        AST x = list3(symbol("+"), symbol("2"), symbol("1"));
        AST y = list3(symbol("+"), symbol("2"), symbol("?y"));
        EnvKV *env = NULL;
        int res = unify(&x, &y, &env);
        mu_assert(res);
        AST sym1 = symbol("1");
        mu_assert(ast_eq(get_types(env, new_symbol("?y")), &sym1));
    }
    {
        AST x = list3(symbol("+"), symbol("?x"), symbol("1"));
        AST y = list3(symbol("+"), symbol("2"), symbol("?y"));
        EnvKV *env = NULL;
        int res = unify(&x, &y, &env);
        mu_assert(res);
        AST sym1 = symbol("1");
        AST sym2 = symbol("2");
        mu_assert(ast_eq(get_types(env, new_symbol("?x")), &sym2));
        mu_assert(ast_eq(get_types(env, new_symbol("?y")), &sym1));
    }
    {
        AST x = list1(symbol("?x"));
        AST y = list1(symbol("?y"));
        EnvKV *env = NULL;
        int res = unify(&x, &y, &env);
        mu_assert(res);
        AST ysym = symbol("?y");
        mu_assert(ast_eq(get_types(env, new_symbol("?x")), &ysym));
    }
    {
        EnvKV *env = NULL;
        AST plus = list3(symbol("+"), symbol(":int"), symbol(":int"));
        AST call = symbol("?t");
        int res = unify(&call, &plus, &env);
        // print_env(env);
        mu_assert(res);
        AST plus2 = list3(symbol("+"), symbol("?x"), symbol("?y"));
        AST call2 = symbol("?t2");
        res = unify(&call2, &plus2, &env);
        mu_assert(res);
        AST plus3 = symbol("?t");
        AST call3 = symbol("?t2");
        res = unify(&call3, &plus3, &env);
        mu_assert(res);

        AST intsym = symbol(":int");
        mu_assert(ast_eq(get_types(env, new_symbol("?x")), &intsym));
    }
}

MU_TEST(test_assign_type_names)
{
    {
        AST x = list4(symbol("+"), symbol("x"), symbol("x"), symbol("y"));
        EnvKV *types = NULL;
        types = assign_type_names(&x, types);
        AST plussym = symbol("+");
        AST xsym = symbol("x");
        mu_assert(ast_eq(types[0].key, &plussym));
        mu_assert(ast_eq(types[1].key, &xsym));
    }
}

MU_TEST(test_generate_constraints)
{
    {
        AST x = list4(symbol("+"), symbol("x"), symbol("x"), symbol("y"));
        EnvKV *types = NULL;
        types = assign_type_names(&x, types);
        // print_types(types);
        Constraint *constraints = NULL;
        constraints = generate_constraints(types, constraints);

        /*++
        for (int i = 0; i < arrlen(constraints); i++) {
            prin_ast(&constraints[i].left);
            printf(" = ");
            print_ast(&constraints[i].right);
        }
        */

        AST t0 = symbol("?t0");
        AST func_types = list5(symbol("?t1"), symbol("?t1"), symbol("?t2"), symbol("->"), symbol("?t3"));
        mu_assert(ast_eq(&arrlast(constraints).left, &t0));
        mu_assert(ast_eq(&arrlast(constraints).right, &func_types));
    }
}

MU_TEST(test_basic_inference)
{
    {
        AST x = list4(symbol("+"), symbol("x"), symbol("x"), symbol("y"));
        EnvKV *types = NULL;
        types = assign_type_names(&x, types);

        Constraint *constraints = NULL;
        constraints = generate_constraints(types, constraints);

        EnvKV *env = NULL;
        for (int i = 0; i < arrlen(constraints); i++)
        {
            int res = unify(&constraints[i].left, &constraints[i].right, &env);
            mu_assert(res);
        }
    }

    {
        AST x = list3(symbol("do"),
                      list3(symbol("+"), symbol("x"), symbol("y")),
                      list3(symbol("+"), symbol("x"), (AST){.ast_type = AST_NUMBER, .number = 10}));
        EnvKV *types = NULL;
        types = assign_type_names(&x, types);

        // print_types(types);

        Constraint *constraints = NULL;
        constraints = generate_constraints(types, constraints);

        /*
        for (int i = 0; i < arrlen(constraints); i++)
        {
            prin_ast(&constraints[i].left);
            printf(" = ");
            print_ast(&constraints[i].right);
        }
        */

        EnvKV *env = NULL;
        for (int i = 0; i < arrlen(constraints); i++)
        {
            int res = unify(&constraints[i].left, &constraints[i].right, &env);
            mu_assert(res);
        }
        // print_env(env);
    }

    {
        char *code = slurp("test/inference/basic.lisp");
        Token *tokens = tokenize(code, strlen(code));
        AST *root_nodes = parse_all(code, tokens);

        EnvKV *env = standard_environment();

        AST *typed_nodes = add_type_all(&env, root_nodes);
        ast_resolve_types_all(env, typed_nodes);

        Constraint *constraints = NULL;
        constraints = generate_constraints(env, constraints);

        /*
        for (int i = 0; i < arrlen(constraints); i++)
        {
            prin_ast(&constraints[i].left);
            printf(" = ");
            print_ast(&constraints[i].right);
        }
        */

        for (int i = 0; i < arrlen(constraints); i++)
        {
            int res = unify(&constraints[i].left, &constraints[i].right, &env);
            mu_assert(res);
        }

        // print_env(env);

        AST *x = &typed_nodes[0].list.elements[2].list.elements[0];
        AST new_type;
        // print_ast(x);
        new_type = *resolve_type(env, x->value_type);
        x->value_type = &new_type;
        AST intsym = symbol(":int");

        // print_ast(&ast[0]);

        mu_assert(ast_eq(x->value_type, &intsym));
    }

    {
        EnvKV *env = NULL;

        {
            AST *type = (AST *)malloc(sizeof(AST));
            *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
            hmput(env, new_symbol("+"), type);
        }

        AST *ast = vajer_ast(slurp("test/inference/basic.lisp"));

        AST *x = &ast[0].list.elements[2].list.elements[0];
        AST *call = &ast[1];
        AST intsym = symbol(":int");
        mu_assert(ast_eq(x->value_type, &intsym));
        mu_assert(ast_eq(call->value_type, &intsym));
    }
}

MU_TEST(test_infer_then_compile)
{
    {
        AST *ast = vajer_ast(slurp("test/inference/adder.lisp"));

        AST *x = &ast[0].list.elements[2].list.elements[0];
        AST intsym = symbol(":int");
        mu_assert(ast_eq(x->value_type, &intsym));

        AST *transformed_nodes = c_transform_all(ast);

        // printf("source:\n%s", c_compile_all(transformed_nodes));
    }
}

MU_TEST(test_infer_var)
{
    {
        AST *ast = vajer_ast(slurp("test/inference/var.lisp"));
        AST *transformed_nodes = c_transform_all(ast);

        // printf("source:\n%s", c_compile_all(transformed_nodes));
    }
}

MU_TEST(test_infer_malloc)
{
    eval(slurp("test/inference/malloc.lisp"));
}

MU_TEST_SUITE(test_suite_inference)
{
    MU_RUN_TEST(test_basic_unification);

    MU_RUN_TEST(test_assign_type_names);

    MU_RUN_TEST(test_generate_constraints);

    MU_RUN_TEST(test_basic_inference);

    MU_RUN_TEST(test_infer_then_compile);

    MU_RUN_TEST(test_infer_var);

    // MU_RUN_TEST(test_infer_malloc);
}