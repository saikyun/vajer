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

* ~~[ ] Implement `list`-macro, it should work like this one, but only be: `list(symbol("do"), list(symbol("var"), symbol(branch1.sym)))`:~~
* ~~[ ] Implement variants like so: `do(var(branch1.sym))`~~
* ~~[ ] Use these instead going forward~~

Alternatively...
```
AST *elements = NULL
arrpush(elements, var(branch1.sym));
do(elements);
```

* Pros: Don't need varargs
* Cons: No nesting... :(

Alternatively, again:
```
AST do_block = list();
arrpush(do_block.list.elements, symbol("do"));
AST var = list();
arrpush(var.list.elements, symbol("var"));
arrpush(var.list.elements, symbol(branch1.sym));
arrpush(do_block.list.elements, var);
```

Alternatively:
```
list2(symbol("do"), list2(symbol("var"), symbol(branch1.sym)));
```
^ pretty clean and helps with most important cases. Very easy to implement

* [x] Implement list1-4, symbol

```
AST *els = NULL;
arrpush(els, ((AST){.type = AST_SYMBOL, .symbol = "do"}));
AST do_block = (AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = els}};

AST *elements = NULL;
arrpush(elements, ((AST){.type = AST_SYMBOL, .symbol = "var"}));
arrpush(elements, ((AST){.type = AST_SYMBOL, .symbol = branch1.sym}));
arrpush(els, ((AST){.type = AST_LIST, .list = (List){.type = LIST_PARENS, .elements = elements}}));
```

----------

It feels like the "normalization" of if-statements becomes a bit clunkier than it has to be. But I think it will work for now. I should probably settle for this for now, and try to generate C code from the new tree. :)

I wonder if the `transform_if` should take the "value to set" as an argument. Like a final form of some sort... That might make sense.

* [x] Write down what C code should be generated
* [x] Start working on some sort of test case
* [x] Implement `c_compile`

Eyy, it works. And now it felt a lot easier than before.

* [x] Fill out tests with more examples (factorial)
* [x] Implement missing stuff
    * [x] Fix funcall

-------------

After a long break I managed to get back to the project. Added some nice emitting, thanks to indentation. Also fixed semicolons, and function calls. Now compiling and running the `fac` function works as expected. :)

There are still things to do though, off the top of my head:

* Types (right now only `int` is supported)
* While-loops
* Structs

---------------

Gonna try explicit types.

```
(defn str-num-printer [str :string num :int]
  (printf "str: %s, num: %d" str num))
```

Currently getting:

```
int str-num-printer(str:stringnum:int) {
  return printf("str: %s, num: %d", str, num);
}
```

Problems:
* [x] - to _
* [x] void should have no return
* [x] comma separated args
* [x] types should be handled properly, perhaps in c-ast as `(char *str)`

Yay, now it can build with some basic types!

```
(defn strlen-plus-n
  [str :string num :int] :int
  (+ num (strlen str)))

# to

(defn strlen_plus_n
  [(:string str) (:int num)] :int
  (return (+ num (strlen str))))

# to

int strlen_plus_n(char* str, int num) {
  return num + strlen(str);
}
```

---------------------

For now, I wanna try to do the basics to run a sdl window, but write it in my lisp.

I'm also a bit tired of writing lispy code inside C-strings, so let's start by figuring out how to "load" a file in C.

* [x] Open a file and turn it into a char *
    * [x] Open a file
    * [x] Read many stuffs from it

I quickly got stuck. Need to compare the load_files I've written to see what's going on.

Managed to solve it. It seems I was trying to read large chunks when I was supposed to read one char at the time, I think.

* [x] `eval`

Managed to run SDL_Init and SDL_Quit!

Next up is creating a window and stuff like that. :)

I'll have to check my kickstart-sdl repo to see what should be done next.

### Do I want def, or let, or something let-like that can hande errors etc?

# TODO: I probably need to solve running transform on arguments of function calls.

# TODO: I need to know types of ast nodes

It seems that for SDL to work on macos, I'm going to need while loops and void if-statements. Gotta fix that then!

# TODO: This code breaks because of `if` wanting to set a parent's (defn's) return value

```
(defn move-up
  [map :char*
   mapw :int
   maph :int] :int
  (var pos :int (- (strchr map 0) map))
  (if (>= (- pos mapw) 0)
    (do
      (put map pos 1)
      (put map (- pos mapw) 0)
      0))
)
```

---------------------------

I've added while loops and somehow void if statements. Now a character can walk around on a map. Pretty neat!

I wonder what the next step is?

Perhaps money. : )

* [x] Add money
    * [x] Randomly put money on map
    * [x] When money is walked on, increment gold counter
    * [x] Print gold counter

What now?

Falling boulders.

* [x] Falling boulders
    * [x] Randomly place boulders
    * [x] When a spot under a boulder is empty, it falls down
    * [x] When it falls down on a player, exit the game




[x] TODO: `(var x :int (if ...))` doesn't work -- in general things are not transformed recursively. need to fix :)

* [x] try running transform recursively

It's kinda better, but `var` needs to be handled separately. I'm also not quite sure how this should work with e.g. args for a function call. How does it work now?

* [x] Write a test for `if` in function arguments

Now it behaves a bit better, but the output is kinda shitty.

* [x] Output the current indentation when in upscope

Funcall should probably not always end up in a gensym. :O
...Or maybe it should.

# TODO: Don't set result of void expressions to a varible :O

I'd need types somewhere, and then code to somehow figure "hey, this type is void, let's not set stuff that is void".

Where can I put types? I'm thinking on AST nodes. Certain things will have clearly defined types, like numbers. I guess numbers are their own type already. I think it's symbols that will need more type information.

~~* [ ] Put .value_type in AST~~

I need some way to resolve the symbol to get the type. I'm thinking something like fetching from a hash map, from symbol to type.

But that gets confusing, since symbols can be shadowed etc. Maybe it's the AST that should have the type after all. I'll try it.

* [x] Put .value_type in AST
* [x] add `add_type_all`
* [x] write test that checks that a type is defined for a `declare`d thing

Okay, the chessboard has been set up.

Now I need to... Add the types somehow.

Easiest? key-value store. One for globals.

* [x] add key-value struct, from symbol to type (...also a symbol)
* [x] add `.globals` to TypeState
* [x] `declare` populates `.globals`

Phew, that seems to work.

Now I need to use this when transforming / compiling C code. Need a break first though.

I think main problem atm is that when doing a funcall, if the result is void, there should be no symbol. Let's try that!

Now this is used when calling functions. Nice!

It seems though like I'll need the same for other types of functions...

* [x] Add `declare` `SDL_Renderer*` for e.g. SDL_CreateRenderer
* [x] Add that type like with void
* [x] When that type is encountered is a funcall, change it from `int` to that type

Okay, I did that, and then there were some stuff with `in` that I had to fix. It's kinda fixed but now `in` has the type of `char**` when it should be type `char*`.

I also need to move things around a bit, so that it's the node that gets the "unique" type, rather than each `in` symbol having a different type.

* [x] Change tests so that the node has the return type, rather than the function
* [x] Fix test
* [x] Write test for: Remove a * at the end for the `in` type (it kinda has the type `T* -> T`)
* [x] Fix test

Okay, phew, some things work better now. But intermediate symbols still are turned into `int` which is not fantastic. Like so:


```clojure
(var symbols :char** (malloc (* 5 (sizeof char*))))
```

turns into

```c
int gensym68;
gensym68 = malloc((5 * sizeof(char*)));

char** symbols = gensym68;
```

I should probably create a smaller test for this.

* [x] Create test to check that the gensym gets type `:void*`
* [x] Fix the test

The game works again!!!

I should probably go through the warnings, I think many of them have to do with int stuff.

* [x] Look at first warning
* [x] Write down a bit what causes it

```
  int gensym37;
  gensym37 = strchr(map, 0);

  char* rrr = gensym37;
```

strchr returns a `char *`, so should probably just declare that.

Fixed. I just needed some `declare`, and make sure that type was used in more places.