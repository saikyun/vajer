# Thought: More Lispy?

I've managed to do the type checking and outputting C code, which I thought was the hard part. Now that that's done, I feel it is time to clean things up. Do the development to my design. Use my judging brain more than my creative brain. Issues that have been festering:

- memory is just very haphazardly allocated and never freed, maybe not super good for a long running repl
- `resolve_types` dies when running multiple times on the same ast, because the state (specifically accumulated gensyms) leads to variable types, `?tx`, overlapping
- in addition to `resolve_types`, I want to make sure all functions included in eval can run incrementally, i.e. one form at the same, but with the same environment (`vajer_ast`, `resolve_types`, `c_ast` (including `c_transform_all`), `c_compile_all`)

Things to figure out:

- who should own data? it feels like `VajerEnv` (as a concept, nothing that's currently in the code) should own all definitions and types
- see if there are problems other than accumulating gensyms which might hinder incremental compilation
  - what happens when the number of types are bigger than an int, or maybe a 64 bit int? maybe that's a lot of types (18446744073709551615x types actually). maybe a 64 bit int is enough for a while
- how slow will it be to create new `tcc_state` all the time? I feel like I wish that I could reuse the parts that are the same (e.g. parsing .h-files), and just add the little bit to the end (the definition of new functions).
  - perhaps the idea of adding all symbols manually works
  - so yeah, need to figure out dependencies for all code
- type errors are hard to understand
  - source mapping disappears too much atm
  - "stack trace" for type constraints
  - user-`declare`d symbols should have "priority", i.e. when something is not aligned with them, it should be clear that they don't match the declaration
- unclear behaviour when inferring struct types when there are multiple structs with the same ... structure (names and types)
- should I keep using the stb_ds.h data structures? the point of them is making C code easier to write, but I don't know if I want to write C, I'd prefer writing lisp, which leads me to:

Perhaps fun thing? :D

- make a lisp! I mean, an actual lisp. it feels like that might be a neat starting point, and have that be how the core of the compiler works. then I can build out more of the compiler using that little lisp instead of hacking in C.
  - what would be needed? let's look at the definition!

```
The S-function apply is defined by
apply[f; args] = eval[cons[f; appq[args]]; NIL],
where
appq[m] = [null[m] → NIL;T → cons[list[QUOTE;car[m]];appq[cdr[m]]]] and
eval[e; a] = [
    atom [e] → assoc [e; a];
    atom [car [e]] → [
        eq [car [e]; QUOTE] → cadr [e];
        eq [car [e]; ATOM] → atom [eval [cadr [e]; a]];
        eq [car [e]; EQ] → [eval [cadr [e]; a] = eval [caddr [e]; a]]; eq [car [e]; COND] → evcon [cdr [e]; a];
        eq [car [e]; CAR] → car [eval [cadr [e]; a]];
        eq [car [e]; CDR] → cdr [eval [cadr [e]; a]];
        eq [car [e]; CONS] → cons [eval [cadr [e]; a]; eval [caddr [e]; a]];
        T → eval [cons [assoc [car [e]; a]; evlis [cdr [e]; a]]; a];
    ];
    eq [caar [e]; LABEL] → eval [cons [caddar [e]; cdr [e]]];
    cons [list [cadar [e]; car [e]; a]];
    eq [caar [e]; LAMBDA] → eval [caddar [e]];
    append [pair [cadar [e]; evlis [cdr [e]; a]; a]]]
and
evcon[c; a] = [eval[caar[c]; a] → eval[cadar[c]; a]; T → evcon[cdr[c]; a]] and
evlis[m; a] = [null[m] → NIL; T → cons[eval[car[m]; a]; evlis[cdr[m]; a]]]
```

^ the above might have some typos, but I think it's all right for now.

So, I'm first gonna translate the above to (imo) more readable:

```
The S-function apply is defined by
apply[f; args] = eval[append[f; appq[args]]; NIL],
where
appq[m] = [null[m] → NIL;T → append[list[QUOTE;first[m]];appq[rest[m]]]] and
eval[e; a] = [
    atom [e] → assoc [e; a];
    atom [first [e]] → [
        eq [first [e]; QUOTE] → second [e];
        eq [first [e]; ATOM] → atom [eval [second [e]; a]];
        eq [first [e]; EQ] → [eval [second [e]; a] = eval [third [e]; a]];
        eq [first [e]; COND] → evcon [rest [e]; a];
        eq [first [e]; FIRST] → first [eval [second [e]; a]];
        eq [first [e]; REST] → rest [eval [second [e]; a]];
        eq [first [e]; APPEND] → append [eval [second [e]; a]; eval [third [e]; a]]; T → eval [append [assoc [first [e]; a]; evlis [rest [e]; a]]; a];
    ];
    eq [first [first [e]]; LABEL] → eval [append [caddar [e]; rest [e]]; append [list [cadar [e]; first [e]; a]]];
    eq [first [first [e]]; LAMBDA] → eval [caddar [e]]; append [pair [cadar [e]; evlis [rest [e]; a]; a]]]
and
evcon[c; a] = [eval[first [first[c]]; a] → eval[cadar[c]; a]; T → evcon[rest[c]; a]] and
evlis[m; a] = [null[m] → NIL; T → append[eval[first[m]; a]; evlis[rest[m]; a]]]
```

Okay I'm jsut confused now. I have some idea what it means, but the `caddar` etc are confusing me. \
`a` seems to be the "environment", all bound variables. \
`e` is the expression. \
`(QUOTE x)` evaluates to `x`. \
`x` evaluates to whatever it is bound to in `a`. \
`(ATOM x)` evaluates to `(ATOM (eval x))`, i.e. a new atom with whatever `x` is bound to. \
`(EQ x y)` evaluates to `(= (eval x) (eval y))`. makes sense. \
`(COND ...stuff)` I believe means something like "eval every other form in stuff, if it is true, eval the form right after that one and return that" \
`(FIRST x)` assume `x` is a list and returns the first element in that list, but I don't quite get it. \
rest same \
`(APPEND x y)` means something like making a list like so: `[x y]`
I think the `T` part might mean you can have more variables, like `(APPEND x y a b)`, but I'm not sure... \
I think `((LABEL x) y)` means putting something in `a`, but I'm not sure. \
Similarly, `LAMBDA` means eval'ing with some stuff at the top of `a`. That makes a lot of sense I guess. :)

Okay, so, the idea now is how to deconstruct this into a basic set of things I could implement, hopefully without too much extra work. I mean, tcc will be doing the actual execution I figure. So it's more about "how few things can I get away with implementing in C". At least I think that's what it's about. I'm thinking something like this:

```
(def x 10)
(def add (fn [x y] (do (+ x y))))
(print (add 5 10))

(list 1 2 3) #> [1 2 3]
```

I guess the immediate question is "where does + come from?". And the answer is that some things are pre-put into the environment.

```
# first element of each entry is the implementation, the second the type.
# the implementation can be vajer-functions earlier compiled
Environment e = {
    +       [cfun_+,        (:int :int -> :int)]
    def     [special_def,   (:symbol :T -> :void)]                          # kind of implicitly takes the environment I guess :)
    fn      [special_fn,    (:symbol [:symbol] :T1 -> (...:T2 -> :T1))]     # not sure how to express the return type
    print   [special_print, ([:char] ...:T -> :void)]                       # macro which compiles the interpolation string during compile time
    list    [special_list,  (...:T -> [...:T])]
    second  [cfun_second,   ([:T1 :T2 ...] -> :T2)]
}
```

With this, I also imagine some sort of special form `comptime`:

```
(def inline-inc (fn [x] (list '+ x 1)))
(def x (comptime (inline-inc 5))) # turns into: `(def x (+ 5 1))`
```

In this case `defmacro` would just be defining a function, and whenever it is called the call is implicitly wraped with `comptime` or something.

How else could I express this?

```
(def x (eval (list '+ 5 1)))
```

Hm, I guess it's a question about when things get run... In the above that works, but what about inside a function?

```
(def f (fn [] (eval (list '+ 5 1))))
```

In this case, I'd like the `eval` part to happen during compile time, but it feels to me like it would be called during runtime. So it kinda needs to be like this:

```
(def f (fn [] (comptime (eval (list '+ 5 1)))))
```

Another way:

```
(def f (eval (list 'fn [] (list '+ 5 1))))
```

Huh, I guess that works.

---

What about structs? Hm.

Structs are just tuples?

```
(def person ["Jona" 15])
(def person-age [p] (second p))
```

Perhaps a special thingamajing:

```
(defstruct Person {:name [:char], :age :int})
#> evaluates into
(def Person.new (fn [name age] [name age]))
(def Person.age (fn [person] (second person)))
(def Person.name (fn [person] (first person)))
```

I also imagine there's an ever-expanding `get`:

```
(get (Person.new "Jona" 15) :age)
#> same as
(Person.age (Person.new "Jona" 15))
#> maybe same as
(def p (Person.new "Jona" 15))
p.age
```

I guess this implies though that different tuples have different types, in order to get the name.
This might not make all that much sense when it's all gonna be compiled to C either.

The `get`-thing makes the most sense when I want the same function to be able to take different types of structs, e.g.

```
(defn damage
  [attacker defender]
  (update defender :hp - (get attacker :damage)))
```

I guess this could work as some sort of macro that knows the type of `attacker`. Hm.

```
(case (typeof attacker)
  Person
  (Person.damage attacker)

  Animal
  (Animal.damage attacker))
```

I mean, why not. I guess it'd have to inline it though.

Might be structs are kinda fine as they are now.

Though maybe it'd make some sense to compile functions using structs to things like

```
struct {float a; float b; float c} v3add(struct {float a; float b; float c} v1, struct {float a; float b; float c} v2) {
    return {v1.a + v2.a, v1.b + v2.b, v1.c + v2.c};
}
```

Hmmmm.

I don't think I'm reaching anything conclusive.

I'm starting to think that for now I'll just keep structs the same, and I don't know if it's worth rewriting too much of the existing stuff just to be closer to og lisp. What I do feel that I want though is `VajerEnv` and having `eval` in that env. And `def` and `defn` I think. When that is done I'd like to figure out if I can express `defn` in terms of `def` and `eval` instead. The goal is being able to express more things in the lisp rather than in the C code.

Soooo... Yeah. I think next step is just creating `VajerEnv` which should carry gensym-numbers and `EnvKV`, and be persisted between `eval`-calls. Let's try that!

Maybe I can then later implement the subset or something. It just made my brain hurt too much, thinking about how to get that together with the C stuff.

---

Started to refactor from `CEntry` and `EnvKV` (the latter is now `TypeKV`) to `VajerEnv`.
Just got to continue hitting M-T (which runs `make test`) and fix compilation errors.
