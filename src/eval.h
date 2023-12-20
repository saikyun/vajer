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
        *type = list3(symbol("?T"), symbol("->"), list1(symbol("?T")));
        hmput(types, new_symbol("ref"), type);
    }
    /*
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list5(symbol(":int"), symbol("?T2"), symbol("?T2"), symbol("->"), symbol(":T2"));
        hmput(types, new_symbol("if"), type);
    }
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol(":int"), symbol("?T2"), symbol("->"), symbol(":T2"));
        hmput(types, new_symbol("if1"), type);
    }
    */
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list3(symbol(":Any"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("sizeof"), type);
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
    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list4(symbol("?T"), symbol("?T"), symbol("->"), value_type_void);
        hmput(types, new_symbol("declare-struct"), type);
    }

    {
        AST *type = (AST *)malloc(sizeof(AST));
        *type = list3(symbol(":int"), symbol("->"), symbol(":int"));
        hmput(types, new_symbol("zero?"), type);
    }

    VajerEnv *env = (VajerEnv *)malloc(sizeof(VajerEnv));
    env->types = types;
    env->values = NULL;
    env->gensym = 0;
    env->global_scope = (Scope){};

    return env;
}

////////////////// AST //////////////////////
//
//
//

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
} ScopeState;

char *scope_gensym(VajerEnv *env, char *sym)
{
    int len = snprintf(NULL, 0, "%d", env->scope_gensym) + strlen(sym) + 10;
    char *str = malloc(len * sizeof(char));
    snprintf(str, len, "__SCOPE%d__%s", env->scope_gensym, sym);
    return str;
}

void _ast_add_scope(VajerEnv *env, ScopeState *state, AST *ast)
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

                env->scope_gensym++;

                for (int i = 0; i < arrlen(args); i++)
                {
                    if (strncmp(args[i].symbol, "__SCOPE", 7) != 0)
                    {
                        shput(state->scope->kvs, args[i].symbol, scope_gensym(env, args[i].symbol));
                    }
                }
            }
            // if state->scope is null, we are in
            else if (elems[0].ast_type == AST_SYMBOL && strcmp(elems[0].symbol, "var") == 0)
            {
                // use the scope for the potential variable
                _ast_add_scope(env, state, &elems[2]);

                env->scope_gensym++;
                if (strncmp(elems[1].symbol, "__SCOPE", 7) != 0)
                {
                    shput(state->scope->kvs, elems[1].symbol, scope_gensym(env, elems[1].symbol));
                }

                // add scope to the new variable name
                _ast_add_scope(env, state, &elems[1]);
                break;
            }
        }

        for (int i = 0; i < arrlen(elems); i++)
        {
            _ast_add_scope(env, state, &elems[i]);
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
            _ast_add_scope(env, state, &kvs[i].value);
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

void ast_add_scopes(VajerEnv *env, AST *ast)
{
    ScopeState state = {.scope = &env->global_scope};
    for (int i = 0; i < arrlen(ast); i++)
    {
        _ast_add_scope(env, &state, &ast[i]);
    }
}

void defmacro(VajerEnv *env, AST *ast)
{
    add_type(&env->macros, &ast->list.elements[1], ast);

    // AST *ast = NULL;
    // arrpush(ast, list3(symbol("printf"), ast_string("hello YEAH!!!\n"), ast_string("")));

    // eval_ast(ast);
}

void resolve_types_eval_ast(VajerEnv *env, AST *ast);

AST *_ast_eval_macros(VajerEnv *root_env, AST *ast)
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
                if (arrlen(root_env->forms_to_compile) > 0)
                {
                    AST *forms = NULL;
                    for (int i = 0; i < arrlen(root_env->forms_to_compile); i++)
                    {
                        arrpush(forms, root_env->forms_to_compile[i]);
                    }

                    arrfree(root_env->forms_to_compile);

                    resolve_types_eval_ast(root_env, forms);
                }

                defmacro(root_env, ast);
                return NULL;
            }
            else
            {
                AST *macro = get_types(root_env->macros, &head);
                if (macro)
                {
                    // if the macro is not in state->values, it has not been compiled yet
                    if (shgeti(root_env->values, head.symbol) == -1)
                    {
                        log("new macro! ");
                        print_ast(&head);
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

                        resolve_types_eval_ast(root_env, new_ast);
                    }

                    if (shgeti(root_env->values, head.symbol) == -1)
                    {
                        log("could not find in root_env: ");
                        print_ast(&head);

                        sai_assert(0);
                    }

                    AST *(*macrocall)(AST *) = shget(root_env->values, head.symbol).cvalue;

                    AST *l = NULL;

                    for (int i = 1; i < arrlen(elems); i++)
                    {
                        arrpush(l, elems[i]);
                    }

                    AST *res = macrocall(l);

                    // TODO: not 100% sure what this means, it was meant to
                    // be used with macros calling macros, but that doesn't seem to work atm anyway
                    return _ast_eval_macros(root_env, res);

                    // sai_assert(0);
                }
            }
        }

        for (int i = 0; i < arrlen(elems); i++)
        {
            AST *res = _ast_eval_macros(root_env, &elems[i]);
            elems[i] = *res;
        }

        break;
    }
    case AST_MAP:
    {
        AstKV *kvs = ast->map.kvs;

        for (int i = 0; i < hmlen(kvs); i++)
        {
            AST *res = _ast_eval_macros(root_env, &kvs[i].value);
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
    for (int i = 0; i < arrlen(ast); i++)
    {
        AST *res = _ast_eval_macros(env, &ast[i]);
        if (res != NULL)
        {
            ast[i] = *res;
            arrpush(env->forms_to_compile, ast[i]);
        }
    }
}

AST *resolve_types(VajerEnv *env, AST *root_nodes, char *code)
{
    int do_print = 0;

    ast_add_scopes(env, root_nodes);

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

void vajer_ast(VajerEnv *env, char *code)
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

    for (int i = 0; i < arrlen(root_nodes); i++)
    {
        AST *l = NULL;
        arrpush(l, root_nodes[i]);
        ast_eval_macros(env, l);
    }

    // since `ast_eval_macros` might already have compiled forms
    // we don't resolve types for `root_nodes` but just
    // the nodes left to compile
    resolve_types(env, env->forms_to_compile, code);
}

//
//
//
//
//

AST *c_ast(VajerEnv *env, AST *ast)
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

    AST *transformed_nodes = c_transform_all(env, ast);

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

CCompilationState *compile_ast_to_file(VajerEnv *env, AST *ast123, char *path)
{
    CCompilationState *res = c_compile_all(c_ast(env, env->forms_to_compile));

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

void eval_ast(VajerEnv *env)
{
    int do_print = 0;

    if (do_print)
    {
        log("\n\e[33m>>> eval ast\e[0m\n");

        print_env(env);

        prn("\e[34m>>> forms_to_compile\e[0m\n");

        for (int i = 0; i < arrlen(env->forms_to_compile); i++)
        {
            print_ast(&env->forms_to_compile[i]);
            prn("\n");
        }
        AST_PRINT_TYPES = 1;
    }

    for (int i = 0; i < arrlen(env->forms_to_compile); i++)
    {
        if (has_unresolved_types(&env->forms_to_compile[i]) && !is_declare(&env->forms_to_compile[i]))
        {
            log("\e[33m>>>>>>>>>>>>> HAS UNRESOLVED TYPES >>>>>>>>\e[0m\n");
            print_ast(&env->forms_to_compile[i]);
            arrpush(env->forms_with_unresolved_types, env->forms_to_compile[i]);
            arrdel(env->forms_to_compile, i);
            i--;
        }
    }

    log("RUNNING c_ast ETC\n");
    CCompilationState *res = c_compile_all(c_ast(env, env->forms_to_compile));

    env->forms_to_compile = NULL;
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

    AstKV *structs = hmget(env->types, &STRUCT_KEY)->map.kvs;

    if (structs)
    {
        for (int i = 0; i < hmlen(structs); i++)
        {
            char *name = structs[i].key.symbol;
            if (shget(env->declarations, name))
            {
                continue;
            }
            AST *kvs = &structs[i].value;
            struct_type_to_string(&str, name + 1, kvs);
            strstr(&str, ";\n\n");
        }
    }

    for (int i = 0; i < shlen(env->values); i++)
    {
        AST *type = get_types(env->types, env->values[i].value.symbol);

        // TODO: maybe it's okay for some types to not be in env->types?
        // comment next three lines and it will fail
        // on gensym things added by compiler

        if (!type)
        {
            type = env->values[i].value.symbol->value_type;
        }

        if (do_print)
        {
            prn("+ %s\t", env->values[i].key);
            print_ast(type);
        }

        // TODO: not sure this should be needed, feels like `get_types` should already return this :o
        type = resolve_type(&env->types, type);

        if (type->ast_type == AST_LIST && arrlen(type->list.elements) > 1)
        {
            print_ast(type);
            // function declaration
            sai_assert(type_to_string(&str, arrlast(type->list.elements)) != 0);

            strstr(&str, " ", env->values[i].key, "(");
            // type_to_string(&str, cenv[i].value.type);

            // ignore -> and return type
            for (int i = 0; i < arrlen(type->list.elements) - 2; i++)
            {
                sai_assert(type_to_string(&str, type->list.elements[i]));

                if (i != arrlen(type->list.elements) - 3)
                {
                    string(&str, ", ");
                }
            }

            strstr(&str, ");\n");
        }
        else
        {
            // var
            type_to_string(&str, *type);
            strstr(&str, " ", env->values[i].key, ";\n");
        }

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
    strstr(&pathstr, "build/eval/eval_ast", gentype(env), ".c");

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

    // source can be empty
    if (res->source.length > 0)
    {
        fputs(res->source.str, f);
    }
    fclose(f);

    sai_assert(tcc_add_file(s, path) != -1);

    int size = tcc_relocate(s, NULL);

    if (size == -1)
    {
        printf("\e[31mfailed compiling: %s\e[0m", path);
        printf("\nother code:\n%s", str.str);
        sai_assert(0);
    }

    void *ptr = malloc(size);

    sai_assert(tcc_relocate(s, ptr) != -1);

    for (int i = 0; i < shlen(res->env); i++)
    {
        EnvKV *entry = &res->env[i];
        if (do_print)
        {
            log("%s (", entry->key);
            prin_ast(res->env[i].value.symbol);
            prn(") is ");
            AST *type = get_types(env->types, res->env[i].value.symbol);
            if (type)
            {
                print_ast(type);
            }
            else
            {
                prn("no type :O\n");
            }
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
    ast_eval_macros(env, ast);
    resolve_types(env, env->forms_to_compile, "");
    return eval_ast(env);
}

void eval(VajerEnv *env, char *code)
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

    for (int i = 0; i < arrlen(root_nodes); i++)
    {

        AST *l = NULL;
        arrpush(l, root_nodes[i]);
        ast_eval_macros(env, l);

        // since `ast_eval_macros` might already have compiled forms
        // we don't resolve types for `root_nodes` but just
        // the nodes left to compile
        log("resolving types for FORMS TO COMPILE WUT\n");
        print_ast_list(env->forms_to_compile);
        resolve_types(env, env->forms_to_compile, code);

        log("stuff\n");
        print_ast_list(env->forms_with_unresolved_types);
        resolve_types(env, env->forms_with_unresolved_types, code);

        for (int i = 0; i < arrlen(env->forms_with_unresolved_types); i++)
        {
            if (has_unresolved_types(&env->forms_with_unresolved_types[i]))
            {
                log(">>>>>>>>>>>>>>>>>> still unresolved...\n");
                print_ast(&env->forms_with_unresolved_types[i]);
            }
            else
            {
                log(">>>>>>>>>>>>>>>>>>>>> not unresolved anymore :)\n");
                print_ast(&env->forms_with_unresolved_types[i]);
                AST *og_forms_to_compile = env->forms_to_compile;
                env->forms_to_compile = NULL;
                arrpush(env->forms_to_compile, env->forms_with_unresolved_types[i]);
                arrdel(env->forms_with_unresolved_types, i);
                i--;
                eval_ast(env);
                env->forms_to_compile = og_forms_to_compile;
            }
        }

        eval_ast(env);
    }
}

void real_eval(VajerEnv *env, char *code)
{
    vajer_ast(env, code);
    eval_ast(env);
}
