#pragma once

#include "lisp.h"

typedef struct Constraint
{
    AST left;
    AST right;
    AST *left_expr;
    AST *right_expr;
} Constraint;

typedef struct InferTypeState
{

} InferTypeState;

void print_env(EnvKV *env)
{
    log("env %ld:\n", shlen(env));
    for (int i = 0; i < shlen(env); i++)
    {
        prin_ast(env[i].key);
        log("\t");
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

AST *_generics_to_specific(TypeNameState *state, AST *t, EnvKV **generic_types)
{
    if (t->ast_type == AST_LIST)
    {
        AST *new_t = (AST *)malloc(sizeof(AST));
        *new_t = (AST){.ast_type = AST_LIST};

        AST generic_type = symbol("?T");

        for (int i = 0; i < arrlen(t->list.elements); i++)
        {
            AST *type = &t->list.elements[i];
            if (ast_eq(type, &generic_type))
            {
                AST *existing = get_types(*generic_types, type);
                if (existing)
                {
                    arrpush(new_t->list.elements, *existing);
                }
                else
                {
                    AST *new_sym = new_symbol(gentype(state));
                    hmput(*generic_types, type, new_sym);
                    arrpush(new_t->list.elements, *new_sym);
                }
            }
            else
            {
                arrpush(new_t->list.elements, *_generics_to_specific(state, type, generic_types));
            }
        }
        return new_t;
    }
    else
    {
        return t;
    }
}

AST *generics_to_specific(TypeNameState *state, AST *t)
{
    EnvKV *generic_types = NULL;
    return _generics_to_specific(state, t, &generic_types);
}

// assigns type names for unknown types
// assigns concrete types like :int for numbers, [:char] for string
void _assign_type_names(TypeNameState *state, AST *e)
{
    switch (e->ast_type)
    {
    case AST_LIST:
    {
        AST *list = e->list.elements;
        if (arrlen(list) > 0 && list[0].ast_type == AST_SYMBOL && strcmp(list[0].symbol, "declare") == 0)
        {
            AST *type = &list[2];
            list[1].value_type = type;
            hmput(state->types, &list[1], type);
        }
        else if (arrlen(list) > 0 && list[0].ast_type == AST_SYMBOL && strcmp(list[0].symbol, "cast") == 0)
        {
            AST *type = &list[1];
            AST *value = &list[2];

            if (value->ast_type == AST_LIST)
            {
                // function call
                if (value->list.type == AST_LIST)
                {
                    for (int i = 1; i < arrlen(value->list.elements); i++)
                    {
                        _assign_type_names(state, &value->list.elements[i]);
                    }
                }
                // vector
                else
                {
                    for (int i = 0; i < arrlen(value->list.elements); i++)
                    {
                        _assign_type_names(state, &value->list.elements[i]);
                    }
                }
            }

            if (value->ast_type == AST_LIST)
            {
                log("HEHE\n");
                print_ast(value);
                AST *ftype = value->list.elements[0].value_type;
                // ftype->list.elements[arrlen(ftype->list.elements) - 1] = *type;
                print_ast(type);
                print_ast(value);
            }

            // list[2].value_type = type;
            // hmput(state->types, &list[2], type);
            hmput(state->types, e, type);
        }
        else
        {
            for (int i = 0; i < arrlen(e->list.elements); i++)
            {
                _assign_type_names(state, &e->list.elements[i]);
            }
            e->value_type = new_symbol(gentype(state));
            hmput(state->types, e, e->value_type);
        }
        break;
    }
    case AST_SYMBOL:
        if (!in_types(state->types, e))
        {
            AST *type = new_symbol(gentype(state));
            e->value_type = type;
            hmput(state->types, e, type);
        }
        else
        {
            AST *t = get_types(state->types, e);
            e->value_type = generics_to_specific(state, t);
        }
        break;
    case AST_NUMBER:
        if (!in_types(state->types, e))
            hmput(state->types, e, &value_type_int);
        break;
    case AST_STRING:
        if (!in_types(state->types, e))
            hmput(state->types, e, value_type_string());
        break;
    case AST_BOOLEAN:
        if (!in_types(state->types, e))
            hmput(state->types, e, &value_type_boolean);
        break;
    default:
        log("unhandled ast for assign_type_names: ");
        print_ast(e);
        sai_assert(0);
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
                // log("unhandled vector: ");
                // print_ast(e);
            }
            else
            {
                AST *list = e->list.elements;
                sai_assert(list[0].ast_type == AST_SYMBOL);
                char *f = list[0].symbol;

                if (strcmp(f, "do") == 0)
                {
                    AST *ret_type = kv.value;
                    AST *last_type = get_types(types, &arrlast(list));

                    arrpush(constraints, ((Constraint){*ret_type, *last_type, e, &arrlast(list)}));
                } /*
                 else if (strcmp(f, "in") == 0)
                 {
                     AST *list_type = (AST *)malloc(sizeof(AST));
                     *list_type = list1(*kv.value);
                     list_type->list.type = LIST_BRACKETS;
                     AST *elem_type = get_types(types, &list[1]);

                     arrpush(constraints, ((Constraint){*list_type, *elem_type}));
                 }
                 else if (strcmp(f, "var") == 0)
                 {
                     AST *sym_type = get_types(types, &list[1]);
                     AST *value_type = get_types(types, &list[2]);
                     arrpush(constraints, ((Constraint){*sym_type, *value_type}));
                 }*/
                else if (strcmp(f, "if") == 0)
                {
                    AST *ret_type = kv.value;
                    {
                        AST *branch_type = get_types(types, &list[2]);
                        arrpush(constraints, ((Constraint){*ret_type, *branch_type, e, &list[2]}));
                    }

                    if (arrlen(list) > 3)
                    {
                        {
                            AST *branch_type = get_types(types, &list[3]);
                            arrpush(constraints, ((Constraint){*ret_type, *branch_type, e, &list[3]}));
                        }
                    }
                }
                else if (strcmp(f, "cast") == 0)
                {
                    //AST *branch_type = get_types(types, &list[2]);
                    //AST *cast_type = &list[1];
                    //arrpush(constraints, ((Constraint){*cast_type, *branch_type}));
                }
                else if (strcmp(f, "defn") == 0)
                {
                    sai_assert(list[1].ast_type == AST_SYMBOL);
                    char *fname = list[1].symbol;
                    sai_assert(list[2].ast_type == AST_LIST);
                    AST *args = list[2].list.elements;
                    AST func_constraint = (AST){.ast_type = AST_LIST};

                    for (int i = 0; i < arrlen(args); i++)
                    {
                        AST arg = args[i];
                        AST *arg_type = get_types(types, &arg);
                        arrpush(func_constraint.list.elements, *arg_type);
                    }

                    arrpush(func_constraint.list.elements, symbol("->"));

                    AST *last_type;

                    if (arrlen(list) == 3)
                    {
                        last_type = &value_type_void;
                    }
                    else
                    {
                        last_type = get_types(types, &arrlast(list));
                    }

                    log("YEEE LAST TYPE\n");
                    print_ast(last_type);

                    sai_assert(last_type);

                    arrpush(func_constraint.list.elements, *last_type);

                    AST *f_type = get_types(types, &list[1]);

                    arrpush(constraints, ((Constraint){*f_type, func_constraint, &list[1], e}));
                }
                else
                {
                    AST *ret_type = kv.value;
                    AST *f_type = list[0].value_type;
                    AST f_call = (AST){.ast_type = AST_LIST};

                    for (int i = 1; i < arrlen(list); i++)
                    {
                        arrpush(f_call.list.elements, *get_types(types, &list[i]));
                    }

                    arrpush(f_call.list.elements, symbol("->"));
                    arrpush(f_call.list.elements, *ret_type);

                    arrpush(constraints, ((Constraint){*f_type, f_call, f_type, e}));
                }
            }
            break;
        }
        case AST_SYMBOL:
            break;
        case AST_NUMBER:
            break;
        case AST_STRING:
            break;
        case AST_BOOLEAN:
            break;
        default:
            log("unhandled ast for generate_constraints: ");
            print_ast(e);
            sai_assert(0);
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
    sai_assert(is_var(v));

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

char *CURRENT_CODE = NULL;

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
            log("in list, could not unify (empty)\n");
            prin_ast(x);
            log(" = ");
            print_ast(y);
            return 0;
        }

        if (arrlen(x->list.elements) != arrlen(y->list.elements))
        {
            log("in list, could not unify (differing length)\n");
            prin_ast(x);
            log(" = ");
            print_ast(y);
            return 0;
        }

        for (int i = 0; i < arrlen(x->list.elements); i++)
        {
            if (unify(&x->list.elements[i], &y->list.elements[i], env) == 0)
            {
                log("in list, could not unify\n");
                prin_ast(&x->list.elements[i]);
                log(" = ");
                print_ast(&y->list.elements[i]);
                return 0;
            }
        }

        return 1;
    }
    else
    {
        log("no matching branch, could not unify\n");
        prin_ast(x);
        log(" = ");
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
    AST *resolved_type = type;
    while (is_var(resolved_type))
    {
        if (!in_types(env, resolved_type))
        {
            log("could not resolve type: ");
            print_ast(resolved_type);
            break;
        }
        else
        {
            resolved_type = get_types(env, resolved_type);
        }
    }

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
                log("could not resolve\n");
                print_ast(&resolved_type->list.elements[i]);
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
        print_ast(ast);
        AST *new_type = resolve_type(env, ast->value_type);
        if (new_type)
        {
            // log(">> resolved type\n");
            // print_ast(ast);
            ast->value_type = new_type;
            // print_ast(ast);
        }
        else
        {
        }
    }
    else
    {
    }

    switch (ast->ast_type)
    {
    case AST_LIST:
    {
        for (int i = 0; i < arrlen(ast->list.elements); i++)
        {
            ast_resolve_types(env, &ast->list.elements[i]);
        }
        // function call
        if (ast->list.type == LIST_PARENS)
        {
            // if function has resolved type
            if (ast->list.elements[0].value_type && ast->list.elements[0].value_type->ast_type == AST_LIST)
            {
                AST *ret_type = ast_last(ast->list.elements[0].value_type);
                if (ret_type)
                    ast->value_type = ret_type;
                else
                    sai_assert(0);
            }
            else if (strcmp(ast->list.elements[0].symbol, "if") == 0)
            {
                AST *ret_type = ast->list.elements[2].value_type;
                if (ret_type)
                    ast->value_type = ret_type;
                else
                    sai_assert(0);
            }
            else if (strcmp(ast->list.elements[0].symbol, "do") == 0)
            {
                AST *ret_type = arrlast(ast->list.elements).value_type;
                if (ret_type)
                    ast->value_type = ret_type;
                else
                    sai_assert(0);
            }
            else if (strcmp(ast->list.elements[0].symbol, "cast") == 0)
            {
                AST *cast_type = &ast->list.elements[1];
                ast->list.elements[2].value_type = cast_type;
                if (cast_type)
                    ast->value_type = cast_type;
                else
                    sai_assert(0);
            }
            else if (strcmp(ast->list.elements[0].symbol, "defn") == 0)
            {
                AST *ret_type = ast->list.elements[1].value_type;
                if (ret_type)
                    ast->value_type = ret_type;
                else
                    sai_assert(0);
            }
            else if (strcmp(ast->list.elements[0].symbol, "declare") == 0)
            {
            }
            else
            {
                log("no type for ");
                print_ast(ast);
                sai_assert(0);
            }
        }
        break;
    }
    case AST_SYMBOL:
        break;
    case AST_NUMBER:
        break;
    case AST_STRING:
        break;
    case AST_BOOLEAN:
        break;
    default:
        log("unhandled ast for ast_resolve_types: ");
        print_ast(ast);
        sai_assert(0);
        break;
    }
}

int line_no(char *code, int pos)
{
    int nof = 1;
    for (int i = 0; i < pos; i++)
    {
        if (code[i] == '\n')
        {
            nof++;
        }
    }
    return nof;
}

void print_source(char *code, AST *ast)
{
    int start = ast->source.start;
    int stop = ast->source.stop;

    if (start > 0 && stop > 0)
    {
        log("line %d: ", line_no(code, start));
        log("%.*s\n", stop - start, code + start);
    }
    else
    {
        log("no source for expr: ");
        print_ast(ast);
    }
}

void ast_resolve_types_all(EnvKV *env, char *code, AST *ast)
{
    // log("\e[36m>>> env before assigning type names\e[0m\n");
    // print_env(env);
    //  EnvKV *types = NULL;
    env = assign_type_names_all(ast, env);
    log("\n\e[41m>>> env after assigning type names\e[0m\n");
    print_env(env);

    Constraint *constraints = NULL;
    constraints = generate_constraints(env, constraints);

    log("\n\e[42m>>> constraints\e[0m\n");
    CURRENT_CODE = code;
    for (int i = 0; i < arrlen(constraints); i++)
    {
        prin_ast(&constraints[i].left);
        log(" = ");
        print_ast(&constraints[i].right);
        int res = unify(&constraints[i].left, &constraints[i].right, &env);
        if (!res)
        {
            if (constraints[i].left_expr && constraints[i].right_expr)
            {
                log("\e[32mexpr:\e[0m\n");
                prin_ast(constraints[i].left_expr);
                log("\n\e[32m=\e[0m\n");
                print_ast(constraints[i].right_expr);

                print_source(code, constraints[i].left_expr);
                print_source(code, constraints[i].right_expr);
            }
        }
        sai_assert(res);
    }
    log("\n");

    log(">> \e[35menv before resolution\e[0m\n");
    // print_env(env);
    for (int i = 0; i < arrlen(ast); i++)
    {
        ast_resolve_types(env, &ast[i]);
    }
}