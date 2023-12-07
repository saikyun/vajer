#include "lisp.h"
#include "infer_types.h"
#include "slurp.h"

/////////////////// Env ///////////////////////////

void add_op(TypeKV **types, char *type, char *op)
{
    AST *type_ast = (AST *)malloc(sizeof(AST));
    *type_ast = list4(symbol(type), symbol(type), symbol("->"), symbol(type));
    hmput(*types, new_symbol(op), type_ast);
}

VajerEnv *standard_environment()
{
    TypeKV *types = NULL;

    add_op(&types, ":int", "+");
    add_op(&types, ":int", "/");
    add_op(&types, ":int", "<");
    add_op(&types, ":int", ">");
    add_op(&types, ":int", "<=");
    add_op(&types, ":int", ">=");
    add_op(&types, ":int", "%");
    add_op(&types, ":int", "!=");
    add_op(&types, ":int", "||");
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("=="), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list3(symbol(":AST"), symbol("->"), symbol(":void"));
        hmput(types, new_symbol("print_ast2"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list3(symbol("?T"), symbol("->"), symbol(":void"));
        hmput(types, new_symbol("assert"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(list1(symbol(":char")), symbol("?T"), symbol("->"), symbol(":void"));
        hmput(types, new_symbol("printf"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("&&"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("-"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol(":int"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("*"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(list1(symbol("?T")), value_type_int, symbol("->"), symbol("?T"));
        hmput(types, new_symbol("in"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list3(list1(symbol("?T")), symbol("->"), value_type_int);
        hmput(types, new_symbol("arrlen"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(list1(symbol("?T")), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("arrpush"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list5(list1(symbol("?T")), value_type_int, symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("put"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("set"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list5(symbol("?T"), symbol("?T2"), symbol("?T3"), symbol("->"), value_type_void);
        hmput(types, new_symbol(":="), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("var"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T2"), symbol("->"), value_type_void);
        hmput(types, new_symbol("while"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("declare"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("declare-var"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("defstruct"), type);
    }

    VajerEnv *env = (VajerEnv *)malloc(sizeof(VajerEnv));
    env->types = types;
    env->values = NULL;
    env->gensym = 0;

    return env;
}

////////////////// AST //////////////////////
//
//
//

typedef struct ScopeKV
{
    char *key;
    char *value;
} ScopeKV;

typedef struct Scope
{
    ScopeKV *kvs;
    struct Scope *children;
    struct Scope *parent;
} Scope;

char *get_in_scopes(Scope *scope, char *k)
{
    char *res = shget(scope->kvs, k);
    if (res)
        return res;
    if (scope->parent)
        return get_in_scopes(scope->parent, k);
    return NULL;
}

typedef struct ScopeState
{
    Scope *scope;
    int gensym;
} ScopeState;

char *scope_gensym(ScopeState *state, char *sym)
{
    int len = snprintf(NULL, 0, "%d", state->gensym) + strlen(sym) + 10;
    char *str = malloc(len * sizeof(char));
    snprintf(str, len, "__SCOPE%d__%s", state->gensym, sym);
    return str;
}

void _ast_add_scope(ScopeState *state, AST *ast)
{
    switch (ast->ast_type)
    {
    case AST_LIST:
    {
        AST *elems = ast->list.elements;

        Scope *parent = 0;

        if (ast->list.type == LIST_PARENS && arrlen(elems) > 0)
        {
            if (elems[0].ast_type == AST_SYMBOL && strcmp(elems[0].symbol, "defn") == 0)
            {
                parent = state->scope;
                Scope *child = &(Scope){.parent = parent};
                state->scope = child;

                AST *args = elems[2].list.elements;

                state->gensym++;

                for (int i = 0; i < arrlen(args); i++)
                {
                    shput(state->scope->kvs, args[i].symbol, scope_gensym(state, args[i].symbol));
                }
            }
        }

        for (int i = 0; i < arrlen(elems); i++)
        {
            _ast_add_scope(state, &elems[i]);
        }

        if (parent)
        {
            state->scope = parent;
        }

        break;
    }
    case AST_MAP:
    {
        AstKV *kvs = ast->map.kvs;

        for (int i = 0; i < hmlen(kvs); i++)
        {
            _ast_add_scope(state, &kvs[i].value);
        }
        break;
    }
    case AST_SYMBOL:
    {
        char *alias = get_in_scopes(state->scope, ast->symbol);
        if (alias)
        {
            ast->symbol = alias;
        }
        break;
    }

    default:
        break;
    }
}

void ast_add_scopes(AST *ast)
{
    ScopeState state = {.scope = &(Scope){}};
    for (int i = 0; i < arrlen(ast); i++)
    {
        _ast_add_scope(&state, &ast[i]);
    }
}

typedef struct MacroState
{
    TypeKV *macros;
    VajerEnv *env;
} MacroState;

void defmacro(MacroState *state, AST *ast)
{
    AST *name = &ast->list.elements[1];

    hmput(state->macros, name, ast);

    // AST *ast = NULL;
    // arrpush(ast, list3(symbol("printf"), ast_string("hello YEAH!!!\n"), ast_string("")));

    // eval_ast(ast);
}

AST *eval_macro(EnvKV *cenv, VajerEnv *env, AST *ast, char *symname, AST *args);

void resolve_types_eval_ast(VajerEnv *env, AST *ast);

AST *_ast_eval_macros(MacroState *state, AST *ast)
{
    switch (ast->ast_type)
    {
    case AST_LIST:
    {
        AST *elems = ast->list.elements;

        if (ast->list.type == LIST_PARENS && arrlen(elems) > 0)
        {
            AST head = elems[0];
            if (head.ast_type == AST_SYMBOL && strcmp(head.symbol, "defmacro") == 0)
            {
                if (arrlen(state->env->forms_to_compile) > 0)
                {
                    VajerEnv *env = standard_environment();

                    for (int i = 0; i < hmlen(state->env->types); i++)
                    {
                        hmput(env->types, state->env->types[i].key, state->env->types[i].value);
                    }

                    resolve_types_eval_ast(env, state->env->forms_to_compile);
                    for (int i = 0; i < shlen(env->values); i++)
                    {
                        // TODO: keep refactoring to env
                        sai_assert(shgeti(state->env->values, env->values[i].key) == -1);
                        shput(state->env->values, env->values[i].key, env->values[i].value);

                        AST *thing = get_types(env->types, new_symbol(env->values[i].key));

                        if (thing)
                        {
                            hmput(state->env->types, new_symbol(env->values[i].key), resolve_type(&env->types, thing));
                        }
                    }

                    arrfree(state->env->forms_to_compile);
                }

                defmacro(state, ast);
                return NULL;
            }
            else
            {
                AST *macro = get_types(state->macros, &head);
                if (macro)
                {
                    // if the macro is not in state->values, it has not been compiled yet
                    if (shgeti(state->env->values, head.symbol) == -1)
                    {
                        AST *new_ast = NULL;

                        AST macrofun = list3(symbol("defn"), symbol(head.symbol), macro->list.elements[2]);
                        for (int i = 3; i < arrlen(macro->list.elements); i++)
                        {
                            arrpush(macrofun.list.elements, macro->list.elements[i]);
                        }

                        AST type = list3(vector(list1(symbol(":AST"))), symbol("->"), vector(list1(symbol(":AST"))));
                        type.list.type = LIST_BRACKETS;
                        AST macrodecl = list3(symbol("declare"), symbol(head.symbol), type);

                        arrpush(new_ast, macrodecl);
                        arrpush(new_ast, macrofun);
                        // arrpush(new_ast, *ast);

                        VajerEnv *env = standard_environment();

                        for (int i = 0; i < hmlen(state->env->types); i++)
                        {
                            hmput(env->types, state->env->types[i].key, state->env->types[i].value);
                        }

                        resolve_types_eval_ast(state->env, new_ast);

                        // shput(state->cenv, head.symbol, shget(cenv, head.symbol));
                    }

                    sai_assert(shgeti(state->env->values, head.symbol) != -1);

                    AST *(*macrocall)(AST *) = shget(state->env->values, head.symbol).cvalue;

                    AST *l = NULL;

                    for (int i = 1; i < arrlen(elems); i++)
                    {
                        arrpush(l, elems[i]);
                    }

                    AST *res = macrocall(l);

                    // TODO: not 100% sure what this means, it was meant to
                    // be used with macros calling macros, but that doesn't seem to work atm anyway
                    return _ast_eval_macros(state, res);

                    // sai_assert(0);
                }
            }
        }

        for (int i = 0; i < arrlen(elems); i++)
        {
            AST *res = _ast_eval_macros(state, &elems[i]);
            elems[i] = *res;
        }

        break;
    }
    case AST_MAP:
    {
        AstKV *kvs = ast->map.kvs;

        for (int i = 0; i < hmlen(kvs); i++)
        {
            AST *res = _ast_eval_macros(state, &kvs[i].value);
            kvs[i].value = *res;
        }
        break;
    }
    case AST_SYMBOL:
    {
        break;
    }

    default:
        break;
    }

    return ast;
}

void ast_eval_macros(VajerEnv *env, AST *ast)
{
    MacroState state = {.env = env};

    for (int i = 0; i < arrlen(ast); i++)
    {
        AST *res = _ast_eval_macros(&state, &ast[i]);
        if (res != NULL)
        {
            ast[i] = *res;
            arrpush(state.env->forms_to_compile, ast[i]);
        }
    }
}

AST *resolve_types(VajerEnv *env, AST *root_nodes, char *code)
{
    int do_print = 0;

    ast_eval_macros(env, root_nodes);

    ast_add_scopes(root_nodes);

    if (do_print)
    {
        log("\n\e[43m>> with scopes\e[0m\n");
        print_ast_list(root_nodes);
        log("\n");
    }

    // AST *typed_nodes = add_type_all(&env, root_nodes);
    ast_resolve_types_all(env, code, root_nodes);

    if (do_print)
    {
        log("\n\e[33m>> with types\n\e[0m");
        print_ast_list(root_nodes);
        prn("\n");
    }

    return root_nodes;
}

AST *vajer_ast(VajerEnv *env, char *code)
{
    int do_print = 0;
    if (do_print)
    {
        log("\e[34m>> code\n\e[0m%s\n", code);
    }

    Token *tokens = tokenize(code, strlen(code));

    AST *root_nodes = parse_all(code, tokens);
    if (do_print)
    {
        log("\n\e[35m>> parsed\n\e[0m");
        print_ast_list(root_nodes);
        log("\n");
    }

    return resolve_types(env, root_nodes, code);
}

//
//
//
//
//

AST *c_ast(AST *ast)
{
    int do_print = 0;

    // this code creates a `vajer$run` function meant to contain all code
    // that can't be run in the top level in C
    /*
    AST run = list3(symbol("defn"), symbol("vajer$run"), vector(list0()));

    for (int i = 0; i < arrlen(ast); i++)
    {
        AST n = ast[i];

        if (n.ast_type == AST_LIST && arrlen(n.list.elements) > 0 && (ast_eq(&n.list.elements[0], new_symbol("defn")) || ast_eq(&n.list.elements[0], new_symbol("def"))))
            continue;

        log("top level form thing\n");
        print_ast(&n);
        arrpush(run.list.elements, n);
        arrdel(ast, i);
        i--;
    }

    if (arrlen(run.list.elements) > 3)
    {
        run.list.elements[1].value_type = new_list(list2(symbol("->"), *arrlast(run.list.elements).value_type));

        log("RUN 1 1 1 1 1 1 1 1 1 1 1\n");
        print_ast(&run);

        AST *list = NULL;
        arrpush(list, run);
        // list = resolve_types(env, list, "");

        arrpush(ast, list[0]);

        // AST *list = NULL;
        // arrpush(list, run);

        // list = resolve_types(env, list, "");
        // list = c_ast(env, list);

        log("RUN\n");
        print_ast(list);
    }
    */

    if (do_print)
    {
        log("\n>> \e[35mAST\e[0m\n");

        for (int i = 0; i < arrlen(ast); i++)
        {
            print_ast(&ast[i]);
        }
        prn(">> \e[35mEnd of AST\e[0m\n");
    }

    AST *transformed_nodes = c_transform_all(ast);

    if (do_print)
    {
        log(">>> transformed nodes\n");
        for (int i = 0; i < arrlen(transformed_nodes); i++)
        {
            print_ast(&transformed_nodes[i]);
        }
        prn("\n");
    }

    return transformed_nodes;
}

////////////////// Eval / compile //////////////////////

CCompilationState *compile_ast_to_file(AST *ast, char *path)
{
    CCompilationState *res = c_compile_all(c_ast(ast));

    FILE *f = fopen(path, "w");
    sai_assert(f != NULL);

    fputs("#include <stdio.h>\n"
          "#include <string.h>\n"
          "#include <SDL2/SDL.h>\n"
          "#include <assert.h>\n"
          "#include <time.h>\n"
          "#include <stdlib.h>\n"
          "#include <alloca.h>\n"
          "#include \"lisp.h\"\n",
          f);
    fputs(res->source.str, f);
    fclose(f);

    return res;
}

CCompilationState *compile_to_file(VajerEnv *env, char *code, char *path)
{
    return compile_ast_to_file(vajer_ast(env, code), path);
}

void eval_ast(VajerEnv *env, AST *ast)
{
    prn("\n");
    log("\n\e[33m>>> eval ast\e[0m\n");

    AST_PRINT_TYPES = 1;
    for (int i = 0; i < arrlen(ast); i++)
    {
        print_ast(&ast[i]);
        prn("\n");
    }

    // resolve_types(env, env->forms_to_compile, "");

    prn("\e[34m>>> forms_to_compile\e[0m\n");

    for (int i = 0; i < arrlen(env->forms_to_compile); i++)
    {
        print_ast(&env->forms_to_compile[i]);
        prn("\n");
    }
    AST_PRINT_TYPES = 1;

    int do_print = 1;

    CCompilationState *res = c_compile_all(c_ast(env->forms_to_compile));

    arrfree(env->forms_to_compile);
    // CCompilationState *res = compile_ast_to_file(ast, "build/evaled.c");

    TCCState *s = tcc_new();
    // tcc_set_options(s, "-bt10");
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    tcc_set_options(s, "-Werror");

    tcc_add_include_path(s, "src");
    tcc_add_include_path(s, "lib");
    tcc_add_include_path(s, "lib/tinycc/lib");
    tcc_define_symbol(s, "VAJER_EVAL", NULL);

    // sai_assert(tcc_add_file(s, "src/lisp.h") == 0);

    if (do_print)
    {
        log("\n\e[36m>>> new symbols\e[0m\n");
        for (int i = 0; i < shlen(res->env); i++)
        {
            prn("%s\n", res->env[i].key);
            print_ast(&res->env[i].value.ast);
        }
    }

    String str = {};

    if (do_print)
    {
        prn("\n\e[35m>>> pre-existing symbols\e[0m\n");
    }

    for (int i = 0; i < shlen(env->values); i++)
    {
        AST *type = get_types(env->types, env->values[i].value.symbol);
        if (do_print)
        {
            prn("+ %s\t", env->values[i].key);
            print_ast(type);
        }

        // TODO: not sure this should be needed, feels like `get_types` should already return this :o
        type = resolve_type(&env->types, type);

        if (type->ast_type != AST_LIST)
        {
            log("\e[31mwhat should be a function type is not:\e[0m ");
            print_ast(type);
            sai_assert(0);
        }

        type_to_string(&str, arrlast(type->list.elements));
        strstr(&str, " ", env->values[i].key, "(");
        // type_to_string(&str, cenv[i].value.type);

        // ignore -> and return type
        for (int i = 0; i < arrlen(type->list.elements) - 2; i++)
        {
            type_to_string(&str, type->list.elements[i]);
            if (i != arrlen(type->list.elements) - 3)
            {
                string(&str, ", ");
            }
        }

        strstr(&str, ");\n");

        if (shgeti(res->env, env->values[i].key) == -1)
        {
            tcc_add_symbol(s, env->values[i].key, env->values[i].value.cvalue);
        }
        else
        {
            log("\e[31msymbol `%s` exists in both env and newly compiled code\e[0m\n", env->values[i].key);
        };
    }

    String pathstr = (String){};
    strstr(&pathstr, "build/eval_ast", gentype(env), ".c");

    char *path = pathstr.str; // "build/eval_ast.c";

    FILE *f = fopen(path, "w");
    sai_assert(f != NULL);

    fputs("#include <stdio.h>\n"
          "#include <string.h>\n"
          "#include <SDL2/SDL.h>\n"
          "#include <assert.h>\n"
          "#include <time.h>\n"
          "#include <stdlib.h>\n"
          "#include <alloca.h>\n"
          "#include \"lisp.h\"\n",
          f);

    if (str.str != NULL)
    {
        if (do_print)
            prn("externs:\n%s\n", str.str);

        fputs(str.str, f);

        // sai_assert(tcc_compile_string(s, str.str) != -1);
    }

    fputs(res->source.str, f);
    fclose(f);

    sai_assert(tcc_add_file(s, path) != -1);

    int size = tcc_relocate(s, NULL);
    sai_assert(size != -1);

    void *ptr = malloc(size);

    sai_assert(tcc_relocate(s, ptr) != -1);

    for (int i = 0; i < shlen(res->env); i++)
    {
        EnvKV *entry = &res->env[i];
        if (do_print)
        {
            log("%s", entry->key);
            prn(" is ");
            print_ast(get_types(env->types, res->env[i].value.symbol));
        }

        entry->value.cvalue = tcc_get_symbol(s, entry->key);

        if (do_print)
            log("ptr: %p\n", entry->value.cvalue);

        if (shgeti(env->values, res->env[i].key) == -1)
        {
            shput(env->values, res->env[i].key, res->env[i].value);
        }
        else
        {
            log("\e[31msymbol `%s` exists in both env and newly compiled code\e[0m\n", env->values[i].key);
        }
    }

    if (shgeti(res->env, "main") != -1)
    {
        if (do_print)
            log("found main\n");
        void *(*f)() = shget(res->env, "main").cvalue;
        f();
    }

    // sai_assert(tcc_run(s, 0, NULL) == 0);
    // return res->env;
}

void resolve_types_eval_ast(VajerEnv *env, AST *ast)
{
    return eval_ast(env, resolve_types(env, ast, ""));
}

void eval(VajerEnv *env, char *code)
{

    eval_ast(env, vajer_ast(env, code));
}

AST *eval_macro(EnvKV *cenv, VajerEnv *env, AST *ast, char *symname, AST *args)
{
    TCCState *s = tcc_new();

    CCompilationState *cres = c_compile_all(c_ast(resolve_types(env, ast, "")));

    int res = tcc_compile_string(s, cres->source.str);
    if (res != 0)
    {
        log("compilation failed, source:\n");
        prn("%s", cres->source.str);
        sai_assert(0);
    }

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
        return NULL;

    // sai_assert(tcc_run(s, 0, NULL) == 0);
    AST *(*f)(AST *args) = tcc_get_symbol(s, symname);
    printf("symname: %s, SYM? %p\n", symname, f);

    return f(args);
}
