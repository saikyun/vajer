# QnA

## What should I do?

I need to:

* [x] Build hello world
* [x] Download tcc
* [x] Try to get libtcc to run .c code for me
* [x] Write down example lisp code that can work as unit tests
* [x] Write unit tests
* [x] Implement unit tests

## I'm in the process of writing unit tests, but I feel stuck! What should I do?

Play some board games.

Then later, try to find a string library, so I can just copy the strings for now. Or maybe I'll just use malloc / strcpy.

Maybe I shouldn't really use a string library, instead I'll just store start and stop.

I could tokenize and parse at the same time, I guess. It's a lisp after all. Why? Less looping. Why not? Potentially uglier code.

Let's try splitted first.

## I can now tokenize some lisp code, including comments and strings. What should I do now?

Next up comes parsing. I should probably start creating lists and stuff, that can then be evaluated.

Oh, and before that, I'll write some new steps:

* [x] Write unit test for parsing
* [x] Implement them

What should I write? Probably I should just take some of the tokenizing examples and extend them to parse as well.

## I don't know if AST root = parse(tokens) should return a pointer or not

Main question is if it should be heap allocated or copied around. I guess heap allocating it makes sense since we won't know how big it ends up.

## I managed to parse the defn example! What now?

Well, try to `eval` it! :D Which would mean compiling to C. :O

* [x] Write unit test for eval
* [x] Implement it
* [x] Write unit test that handles more forms
* [ ] Add gensym to CompilationState
* [ ] Implement it
* [ ] Probably need some ways to figure out types
* [ ] If nothing else, try filling in the types, like `:int`

I managed to do basic eval!

Now I need to handle certain things though.

Storing the result of if-statements, so that it can be e.g. returned or used in other expressions

## How?

Since the usage depends on the surrounding context, it might make sense to have some sort of handle that can be used if one wants to use the "return value". I think no code should run as an actual expression in C, or as little as possible. Better to do it in a procedural way, so I don't have to think so much about which context one is in (expression or not).

For example:

```
(* (+ 10 10) 20)

// should lead to code like

int res = 10 + 10;
int res2 = res * 20;

// rather than
((10 + 10) * 20)

// because that will make it more compatible with e.g.
int res;
if (1) {
    res = 1;
} else {
    res = 0;
}
```

I'll need a gensym thing of some sort. Just an int for now.

## I feel very confused and strained about turning everything into declarations. How can I solve this?

I have gone through the code and written down an example of how I want the end result to look. Now I should think about if my CompilationResult will cover all these cases.

It might also be wise to "reboot" with a smaller code sample.

I also think I should add some nice string functions for now... :P Specifically "concat", that takes __VA_ARGS__ so that I can append many things.

* [x] Implement strings
* ~~[ ] Start using the strings with declarations etc~~

Maybe I should look through cgen/cjanet to see how bakpakin solves these problems...

Maybe have another step between. cgen assumes c-ast essentially. So one solution could be to convert the lisp ast to a c ast, then emit code. This would be more clean, I think.

```
(if (<= n 1) 1 (* n (fac (- n 1))))
```

would turn into something like:

```
(def res: int)
(do (def cond-res (<= n 1))
    (if cond-res
        (set res 1)
        (do (def reee (* n (fac (- n 1))))
            (set res reee))))
```

* ~~[ ] Make test for the above~~
* ~~[ ] Implement test~~

Also needs to handle "if in if":

```
(if (<= (if (== 0 n) 1337 0) 1) 1 (* n (fac (- n 1))))
```

```
(def outer-res: int)
(do (def res0: int)
    (def cond-res0: int)
    (do (def res1: int)
        (def cond-res1 (== 0 n))
            (if cond-res1
                (set res1 1337)
                (set res1 0))
        (set cond-res0 res1))
    (if cond-res
        (set res0 1)
        (do (def reee (* n (fac (- n 1))))
            (set res0 reee)))
    (set outer-res res0))
```

So a do block declares a return variable, which can be used later to e.g. set values in an if. Then the consumer of the do-block decides what to do with the variable. Sort of like a callback.

I think I should start with doing the c-ast => c stuff.

* ~~[ ] Write some C code, figure out how the ast would look.~~

```
int fac(int n)
{
    int res;
    if (n <= 1)
    {
        res = 1;
    }
    else
    {
        res = n * fac(n - 1);
    }
    return res;
}
```

How would an ast look? Something like this:
```
{
    .type = int,
    .node_type = func,
    .name = "fac",
    .args = {{.type = int, .name = "n", .node_type = argument}},
    .body = {
        {.type = int, .node_type = declaration, .name = "res"},
        {.type = int,
         .node_type = if,
         .cond = {
            .type = int,
            .node_type = expression,
            .body = {
                {.node_type = symbol, .name = "n"},
                {.node_type = symbol, .name = "<="},
                {.node_type = number, .value = 1},
            }
         }
    }
}
```

Hmm, I'm starting to become unsure about two things:

1. The ast -> c can yield strings/structures that doesn't understand anything, e.g. what type a symbol is
2. Maybe there is a way to "unwind" arbitrary lisp code -- the main problem I'm seeing is turning expressions into statements, but maybe this can be "generalized" in some way. What is actually happening when something is used as an expression? The "return value" just needs to be passed to the encompassing form.

```clojure
(defn fib [n]
  (if (<= n 1)
    1
    (* n (fib (- n 1)))))

(def n 10)
(do (def x (if (<= n 1) 1 (* n n)))
    (print x))
```

How to turn this into code like this:

```
(defn fib [n]
  (var res)
  (if (<= n 1)
    (set res 1)
    (set res (* n (fib (- n 1)))))
  (return res))

(def n 10)
(do (var res)
    (if (<= n 1) (set res 1)
                 (set res (* n n)))
    (def x res)
    (print x))
```

1. `defn` has an implicit "call return at end of body"
2. you can lift out `res`, and then you need to pass `res` to the "wrapping form" (e.g. `(def x ...)` or `(return ...)`
3. somehow, when compiling the `def` form, you need some way to say "do this before this happens", and it needs to work on multiple depths of nesting

Let's try some more nested forms.

```clojure
(def n 10)
(do (def x (if (<= n 1) (+ 10 (if (zero? n) 0 1))
                        (* n n)))
    (print x))
```

inbetween

```clojure
(def n 10)
(do (def #1 (if (<= n 1) (+ 10 (if (zero? n) 0 1))
                         (* n n)))
    (def x #1)
    (print x))
```

inbetween2

```clojure
(def n 10)
(do (def #2 (if (zero? n) 0 1))
    (def #1 (if (<= n 1) (+ 10 #2)
                         (* n n)))
    (def x #1)
    (print x))
```

This is a way of recursively normalizing expressions into statements.

Then I need to convert from `(def #2 ...)` to:

```clojure
(def n 10)
(var res)
(do (var #2)
    (if (zero? n) (set #2 0) (set #2 1))
    (var #1)
    (if (<= n 1) (set #1 (+ 10 #2))
                         (set #1 (* n n)))
    (def x #1)
    (set res (print x)))
```

I think this could work! Now the question is still if this should be one step (from lisp ast -> c source), or if I should do the transformation first. Perhaps I should do the transformation first, but it could potentially be done with the AST I've already defined.

What is needed?

* Some sort of "checkpoint", i.e. "where can I place code that is outside of an expression?" This place is prepended to.

1. Traverse forms one by one. When a new do-block is encountered, push a new checkpoint to the stack.
2. When encountering a statement, like `if`, compile it to the checkpoint, get a symbol in return, and substitute the `if` expression for that symbol.
3. The last form of a do-block should always be substituted. I.e. wrap the final element in `(set res ...)`.

Do the simplest variant first:

```
[(if (zero? n) 0 1)]
```

to

```
[(var res)
 (if (zero? n) (set res 0) (set res 1))
 # res somehow accessible, like getting an expr back from compile
 ]
```

---------------------------------------

I think it's starting to look like something now (ignoring the second case of if statements:
```
(if (zero? n) 0 1)
=>
(do (var gensym0) (if (zero? n) (set gensym0 0) (set gensym0 1)))

(if (zero? n) (do (print "hello") (+ 1 2 3)) 1)
=>
(do (var gensym0) (if (zero? n) (do (var gensym1) (do (print "hello") (set gensym1 (+ 1 2 3))) (set gensym0 gensym1)) (set gensym0 1)))
```

The code is really unwieldy atm though, so wanna try to make convenience functions for things like `do`, `var` and `set`. That ought to be pretty easy with variants of my `list`-macro though. :) It's at times like this that it would be nice if I could use macros... :P Soon!