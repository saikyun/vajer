#pragma once

#include "lisp.h"

typedef struct EnvKV
{
    AST *key;
    AST *value;
} EnvKV;

typedef struct Constraint
{
    AST left;
    AST right;
} Constraint;

typedef struct InferTypeState
{

} InferTypeState;

void print_env(EnvKV *env)
{
    printf("env %ld:\n", shlen(env));
    for (int i = 0; i < shlen(env); i++)
    {
        prin_ast(env[i].key);
        printf("\t");
        print_ast(env[i].value);
    }
}

typedef struct TypeNameState
{
    EnvKV *types;
    int gensym;
} TypeNameState;

char *gentype(TypeNameState *state)
{
    int len = snprintf(NULL, 0, "%d", state->gensym) + 2 + 1;
    char *str = malloc(len * sizeof(char));
    snprintf(str, len, "?t%d", state->gensym);
    state->gensym++;
    return str;
}

int in_types(EnvKV *types, AST *e)
{
    for (int i = 0; i < hmlen(types); i++)
    {
        if (ast_eq(types[i].key, e))
            return 1;
    }
    return 0;
}

AST *get_types(EnvKV *types, AST *e)
{
    for (int i = 0; i < hmlen(types); i++)
    {
        if (ast_eq(types[i].key, e))
            return types[i].value;
    }
    return NULL;
}

void _assign_type_names(TypeNameState *state, AST *e)
{
    switch (e->ast_type)
    {
    case AST_LIST:
        for (int i = 0; i < arrlen(e->list.elements); i++)
        {
            _assign_type_names(state, &e->list.elements[i]);
        }
        hmput(state->types, e, new_symbol(gentype(state)));
        break;
    case AST_SYMBOL:
        if (!in_types(state->types, e))
        {
            AST *type = new_symbol(gentype(state));
            e->value_type = type;
            hmput(state->types, e, type);
        }
        else
        {
            e->value_type = get_types(state->types, e);
        }
        break;
    case AST_NUMBER:
        if (!in_types(state->types, e))
            hmput(state->types, e, new_symbol(":int"));
        break;
    default:
        printf("unhandled ast for assign_type_names: ");
        print_ast(e);
        assert(0);
        break;
    }
}

EnvKV *assign_type_names(AST *e, EnvKV *types)
{
    TypeNameState state = {.types = types};
    _assign_type_names(&state, e);
    return state.types;
}

EnvKV *assign_type_names_all(AST *exprs, EnvKV *types)
{
    TypeNameState state = {.types = types};
    for (int i = 0; i < arrlen(exprs); i++)
    {
        _assign_type_names(&state, &exprs[i]);
    }
    return state.types;
}

Constraint *generate_constraints(EnvKV *types, Constraint *constraints)
{
    for (int i = 0; i < hmlen(types); i++)
    {
        EnvKV kv = types[i];
        AST *e = kv.key;
        switch (e->ast_type)
        {
        case AST_LIST:
        {
            if (e->list.type == LIST_BRACKETS)
            {
                // TODO: maybe handle vector?
                printf("unhandled vector: ");
                print_ast(e);
            }
            else
            {
                AST *list = e->list.elements;
                assert(list[0].ast_type == AST_SYMBOL);
                char *f = list[0].symbol;

                if (strcmp(f, "do") == 0)
                {
                    AST *ret_type = kv.value;
                    AST *last_type = get_types(types, &arrlast(list));

                    arrpush(constraints, ((Constraint){*ret_type, *last_type}));
                }
                else if (strcmp(f, "defn") == 0)
                {
                    assert(list[1].ast_type == AST_SYMBOL);
                    char *fname = list[1].symbol;
                    assert(list[2].ast_type == AST_LIST);
                    AST *args = list[2].list.elements;
                    AST func_constraint = (AST){.ast_type = AST_LIST};

                    for (int i = 0; i < arrlen(args); i++)
                    {
                        AST arg = args[i];
                        AST *arg_type = get_types(types, &arg);
                        arrpush(func_constraint.list.elements, *arg_type);
                    }

                    arrpush(func_constraint.list.elements, symbol("->"));

                    AST *last_type = get_types(types, &arrlast(list));

                    arrpush(func_constraint.list.elements, *last_type);

                    AST *f_type = get_types(types, &list[1]);

                    arrpush(constraints, ((Constraint){*f_type, func_constraint}));
                }
                else
                {
                    AST *ret_type = kv.value;
                    AST *f_type = get_types(types, &list[0]);

                    AST f_call = (AST){.ast_type = AST_LIST};

                    for (int i = 1; i < arrlen(list); i++)
                    {
                        arrpush(f_call.list.elements, *get_types(types, &list[i]));
                    }

                    arrpush(f_call.list.elements, symbol("->"));
                    arrpush(f_call.list.elements, *ret_type);

                    arrpush(constraints, ((Constraint){*f_type, f_call}));
                }
            }
            break;
        }
        case AST_SYMBOL:
            // arrpush(constraints, ((Constraint){*e, symbol(kv.value)}));
            break;
        case AST_NUMBER:
            // arrpush(constraints, ((Constraint){*e, symbol(kv.value)}));
            break;
        default:
            printf("unhandled ast for generate_constraints: ");
            print_ast(e);
            assert(0);
            break;
        }
    }
    return constraints;
}

int is_var(AST *e)
{
    return e->ast_type == AST_SYMBOL && e->symbol[0] == '?';
}

int occurs_check(AST *x, AST *y, EnvKV **env)
{
    return 0;
}

int unify(AST *x, AST *y, EnvKV **env);

int unify_variable(AST *v, AST *x, EnvKV **env)
{
    assert(is_var(v));

    if (in_types(*env, v))
    {
        AST *res = get_types(*env, v);
        return unify(res, x, env);
    }
    else if (is_var(x) && in_types(*env, x))
    {
        AST *res = get_types(*env, x);
        return unify(v, res, env);
    }
    else if (occurs_check(v, x, env))
        return 0;
    else
    {
        hmput(*env, v, x);
        return 1;
    }
}

int unify(AST *x, AST *y, EnvKV **env)
{
    if (ast_eq(x, y))
        return 1;
    else if (is_var(x))
        return unify_variable(x, y, env);
    else if (is_var(y))
        return unify_variable(y, x, env);
    else if (x->ast_type == AST_LIST && y->ast_type == AST_LIST)
    {
        if (arrlen(x->list.elements) == 0 && arrlen(y->list.elements) == 0)
        {
            printf("in list, could not unify\n");
            prin_ast(x);
            printf(" = ");
            print_ast(y);
            return 0;
        }

        if (arrlen(x->list.elements) != arrlen(y->list.elements))
        {
            printf("in list, could not unify\n");
            prin_ast(x);
            printf(" = ");
            print_ast(y);
            return 0;
        }

        for (int i = 0; i < arrlen(x->list.elements); i++)
        {
            if (unify(&x->list.elements[i], &y->list.elements[i], env) == 0)
            {
                printf("in list, could not unify\n");
                prin_ast(x);
                printf(" = ");
                print_ast(y);
                return 0;
            }
        }

        return 1;
    }
    else
    {
        printf("no matching branch, could not unify\n");
        prin_ast(x);
        printf(" = ");
        print_ast(y);
        return 0;
    }
}

AST infer_types(InferTypeState *state, AST *node)
{
    return *node;
}

AST *infer_types_all(AST *nodes)
{
    InferTypeState state = {};
    AST *new_nodes = NULL;

    for (int i = 0; i < arrlen(nodes); i++)
    {
        arrpush(new_nodes, infer_types(&state, &nodes[i]));
    }

    return new_nodes;
}

AST *resolve_type(EnvKV *env, AST *type)
{
    if (!in_types(env, type))
    {
        printf("could not resolve type\n");
        // print_ast(type);
        return NULL;
    }
    AST *resolved_type = get_types(env, type);
    if (resolved_type->ast_type == AST_LIST)
    {
        AST *new_type = (AST *)malloc(sizeof(AST));
        *new_type = (AST){.ast_type = AST_LIST};

        for (int i = 0; i < arrlen(resolved_type->list.elements); i++)
        {
            AST *res_type = resolve_type(env, &resolved_type->list.elements[i]);
            if (res_type)
            {
                arrpush(new_type->list.elements, *res_type);
            }
            else
            {
                arrpush(new_type->list.elements, resolved_type->list.elements[i]);
            }
        }

        return new_type;
    }
    else
    {
        return resolved_type;
    }
}

void ast_resolve_types(EnvKV *env, AST *ast)
{
    if (ast->value_type != NULL)
    {
        if (is_var(ast->value_type))
        {
            AST *new_type = resolve_type(env, ast->value_type);
            if (new_type)
            {
                ast->value_type = new_type;
            }
        }
    }

    switch (ast->ast_type)
    {
    case AST_LIST:
    {
        for (int i = 0; i < arrlen(ast->list.elements); i++)
        {
            ast_resolve_types(env, &ast->list.elements[i]);
        }
        break;
    }
    case AST_SYMBOL:
        break;
    case AST_NUMBER:
        break;
    default:
        printf("unhandled ast for ast_resolve_types: ");
        print_ast(ast);
        assert(0);
        break;
    }
}

void ast_resolve_types_all(EnvKV *env, AST *ast)
{
    for (int i = 0; i < arrlen(ast); i++)
    {
        ast_resolve_types(env, &ast[i]);
    }
}