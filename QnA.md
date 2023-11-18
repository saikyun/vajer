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

[x] DONE: I probably need to solve running transform on arguments of function calls.

[x] DONE: I need to know types of ast nodes

It seems that for SDL to work on macos, I'm going to need while loops and void if-statements. Gotta fix that then!

[x] done: This code breaks because of `if` wanting to set a parent's (defn's) return value

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




[x] done: `(var x :int (if ...))` doesn't work -- in general things are not transformed recursively. need to fix :)

* [x] try running transform recursively

It's kinda better, but `var` needs to be handled separately. I'm also not quite sure how this should work with e.g. args for a function call. How does it work now?

* [x] Write a test for `if` in function arguments

Now it behaves a bit better, but the output is kinda shitty.

* [x] Output the current indentation when in upscope

Funcall should probably not always end up in a gensym. :O
...Or maybe it should.

[x] done: Don't set result of void expressions to a varible :O

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

------------

Would be nice if `if` didn't need random `0` at the end. I think this might need type inference stuff. Or possible explicit types first.

* [x] Write a test for if that returns single type
* [x] Fix test


### TODO: this code puts return in wrong place:

```
(declare malloc :void*)

(defn main [] :int
  (if 1 (malloc 1))

  (if 0 (malloc 1) (malloc 1337))

  (var lul :int (if 0 1 2)))
```

--------------

Strange bug with weird data. :O

* [x] Run only the failing test
* [x] Write a function that can print the ast, with decent indentation and types

PHEW

Now the game builds without stupid things like `(if x (do (void-func) 0))`!

The code looks quite a bit like c but lisp. And that's great! That has been my goal for a while.

I've also gotten to think about the types for a while, so I think I have some more ideas about what to do for type inferencing.

I want to represent types using my ASTNode thing instead of chars, main reason being that some types, like function types, are better described as a list. I also don't want to represent lists as `:thing*`, but rather something like `[:thing]` or similar.

I think the better printer helped me see the ast more clearly, so that was nice. It was fun to write as well.

I think it will be helpful for when doing inferencing too.

* [ ] Copy a function from sdl-test and write a "optimal" / ergonomic way of writing the same function

Current:
```clojure
(defn draw_symbol
  [renderer :SDL_Renderer*
   symbols  :char**
   symbol_i :char
   x        :int
   y        :int] :void
  (var symbol :char* (in symbols symbol_i))
  (var w :int 8)
  (var h :int 7)
  (var c :char*)
  (var i :int 0)
  (var j :int 0)
  (while (< j h)
    (while (< i w)
      (set c (+ symbol (+ i (* w j))))
      (if (== *c '.')
        (SDL_RenderDrawPoint renderer (+ x i) (+ y j))
      )
      (set i (+ i 1))
    )
    (set i 0)
    (set j (+ j 1))
  )
)
```

Nice:
```clojure
(defn draw-symbol [renderer symbol {x, y}]
  (let [w 8, h 7]
    (loop [xi :range [0 w]
           yi :range [0 h]
           :let [c (in symbol (+ xi (* yi w)))]
           :when (= c '.')]
      (sdl/render-draw-point renderer (+ x xi) (+ y yi))
    )
  )
)
```

Not sure if I want `(let [...] ...)` or `(def [...]) ...`

Not sure if I want `x :let y` or `:let [x y]`.

All in all I'm pretty happy with the new version of the function. What would definitely be needed are:
* Type inference
* Macros (though as far as I can see, the macros don't need to know types beforehand)

The idea is also that types could be explicitly written, but inference should deal with this. + says x and y are numbers. `in` says `symbol` is a list of some sort, `sdl/render-draw-point` says `renderer` is a `SDL_Renderer*`.

Could I make it even nicer?

The loop is kinda explicit maybe, some way of looping over x/y might make sense. Or over a list but with width and height and get the elements.

```clojure
(defn draw-symbol [renderer symbol {x y}]
  (coord-loop symbol 8
    (fn [c {:x xi, :y yi}]
      (if (= c '.')
        (sdl/render-draw-point renderer (+ x xi) (+ y yi)))))
)
```

Something like this might work. Like an indexed map but with coordinates rather than an index. I think this would be defined as a user though, not the core library.

Let's write one example where I don't have the macros/structs:
```clojure
(defn draw-symbol [renderer symbol x y]
  (var w 8)
  (var h 7)
  (var xi 0)
  (var yi 0)
  (while (< yi h)
    (while (< xi w)
      (var c (in symbol (+ xi (* yi w))))
      (if (= c '.')
        (sdl/render-draw-point renderer (+ x xi) (+ y yi)))
      (set xi (inc xi))
    )
    (set yi (inc yi))
  ))
```

What's missing?
* Types on the args
* Types on var

How to fix types?

For the vars, the w h xi yi are easy, because I have a literal right there. I could just make the rule "the var symbol gets the type of the next expression".

It's harder for c though. It would need to keep the type of symbol in mind, and figure out it's own type when symbol has its type.

Which leads to the arguments. renderer would have to infer from render-draw-point. x and y from +. symbol is the most abstract, I guess it would have to infer through c through the type of comparing to '.', which might be "any".

I have to think a bit about how I'd like the C code to look as well. Let's try writing it out.

```
void draw_symbol(SDL_Renderer *renderer, List symbol, int x, int y) {
  int w = 8;
  int h = 7;
  int xi = 0;
  int yi = 0;
  while (yi < h) {
    while (xi < w) {
      void *c = list_in(symbol, (xi + (yi * w)));
      if (eq(c, '.')) {
        SDL_RenderDrawPoint(renderer, (x + xi), (y + yi));
      }
      xi = xi + 1;
    }
    yi = yi + 1;
  }
}
```

Main question here is equality. Should it use c's ==, or should I define it myself? The alternative would be something like this:

```
void draw_symbol(SDL_Renderer *renderer, List symbol, int x, int y) {
  int w = 8;
  int h = 7;
  int xi = 0;
  int yi = 0;
  while (yi < h) {
    while (xi < w) {
      char c = (char)list_in(symbol, (xi + (yi * w)));
      if (c == '.') {
        SDL_RenderDrawPoint(renderer, (x + xi), (y + yi));
      }
      xi = xi + 1;
    }
    yi = yi + 1;
  }
}
```

I guess it doesn't matter that much. It mostly matters in the sense that either I can have some generic types, that fulfill an interface (such as "comparable"), or I must figure out the type before generating this code. Either when the code is called, checking the type of the list then, or explicitly expressing the type in the defn call.

I think I can punt on that issue for a bit, and just go ahead with some basic type inference for now. Perhaps doing a little at the time.

* [x] Write a test that should be type inferenced
* [ ] Implement it

I've managed to do some basic inferencing, but I think I should go through the tests on here as well: https://github.com/eliben/paip-in-clojure/blob/master/src/paip/11_logic/unification_test.clj

* [x] Write the first test and try to run it

I wrote a bunch of the tests, and now things works. I messed up a bit because returning 0 from unify meant both failure and just that the hashmap was empty. I changed this to use an out variable instead.

Now I need to do the rest of the steps here https://eli.thegreenplace.net/2018/type-inference/

> 1. Assign symbolic type names (like t1, t2, ...) to all subexpressions.
> 2. Using the language's typing rules, write a list of type equations (or constraints) in terms of these type names.
> 3. Solve the list of type equations using unification.

Go through all the stuff and assign type names.

* [*] Write a test for `assign_type_names` that returns a hashmap from char* symbol to AST
* [*] Implement it

The function isn't really complete, but it's a start. Now I need to generate the list of type equations.

```
+               ?t0
x               ?t1
y               ?t2
(+ x x y)       ?t3
```

Should become:
```
(get +) = ((get x) (get x) (get y) -> ?t3)
v
?t0 = (?t1 ?t1 ?t2 -> ?t3)
```

* [x] Implement the above

```
+ = ?t0
x = ?t1
y = ?t2
?t0 = (?t1 ?t1 ?t2 -> ?t3)
```


# TODO: In add_type, AST_LIST value type -- maybe this shouldn't be null?

* [x] Run unification on the result


Eyy it works on some fancy stuff now (test/inference/adder.lisp).

Next step is doing the C transformation and compilation steps. My main problem atm is figuring out if the types should live in the value_type or next to the symbols. E.g.:

```
(var x :int 10)
```
or this with x being :int implicitly
```
(var x 10)
```

I think that it might make sense to transform the code into the first variant. Another question though is how explicit types should look.

```
(defn adder
  [x :int y :int] :int
  (+ x y))
```

This doesn't really work, because :int could be a value rather than a type. Solutions:
1. Move return type before the arguments
2. Make special syntax for types
3. Connect types with the symbol e.g. `x:int`

```
(defn adder :int
  [x :int y :int]
  (+ x y))
```

Other situation:

```
(var x :int 10)
```

I guess in this case it could just see that the number of arguments are 4.
What about multiple declarations?

```
(var x :int 10
     y :int 20)
```

This wouldn't really work. Though I don't know if you ever want explicit types on `var`s. Maybe one would do it with casting instead.

```
(var x (cast 5.2 :int))
```

That looks all right.

As for special syntax, I like : for types, but it's a classic to keep those as keywords. Clojure does the ^int, which I think looks weird. Let's just try some random alternatives:

```
(declare adder [:int :int] :int)
(defn adder
  [x y]
  (+ x y))
```

Types declared separately, haskell style.

```
(defn adder
  [x ^int y ^int] ^int
  (+ x y))
```

Clojure look, but clojure has the type come before the symbol.

```
(defn adder
  [x \int y \int] \int
  (+ x y))
```

I think I just kinda like the idea of types being keywords. Nothing mystical about them. Another question then, is how to express more complicated types.

```
# list
:int[]
:[int]
:list<int>
[:int]

set
#{:int}

union
@{:int | :string}

dictionary
{:int age, :string name, [:Cat] cats}
{age :int, name :string, cats [:Cat]}
{age :int, name :string, cats :[Cat]}
```

I think for now I want it to convert to the explicit types, so let's just do that.

* [x] Write a test that tests for transforming an inferred function to an explicit one
* [x] Implement the transformation

Now it works!

```
(defn adder
  [x y]
  (+ x y))

(defn main []
  (adder 10 20))
```

to

```
int adder(int x, int y) {
  return (x + y);
}

int main() {
  int gensym0;
  gensym0 = adder(10, 20);

  return gensym0;
}
```

Only problem now is that I can't express explicit types. Hm. (Because I ended up using the .value_type instead). So either I need to put .value_type as explicit, or move explicit types into .value_type and remove them from the ast.

Maybe I'll just do the `declare` thing.

# TODO: search for TODO in project :o


.........I've gotten a bit but getting kinda stuck on the in / var stuff. I think it has to do with me putting the types into `env`, and those types not expanding properly.

* [x] Write test for `var` specifically, in type inference stuff

More stuff works.

# TODO: Fix scopes, e.g. this breaks atm:

```
(var str "hello")
(defn x [str] (+ str 1))
```

Hm, atm things feels annoying. I'm gonna try to move things so I can see all the steps in a single function.

* [x] Move all Vajer transformations etc to a single function, that takes char * and outputs AST
* [x] Make a similar function for C, that takes AST and outputs AST *
* [x] Then ast -> c code

They should be called:
AST *vajer_ast(char *code)
AST *c_ast(AST *ast)
char *compile_c(AST *ast)

Done!

Now, the first question is:

## What does add_type_all actually do?

It seems to "infer" types in a "dumb" way, i.e. it can only take concrete types and put onto if, defn etc. This was used when there was no inference. Now this should probably instead be done by some part of resolve_types_all instead.

* [x] Fix malloc and cast

I removed add_type_all because it didn't seem useful anymore, add typename could do its job.

All tests except sdl-test run correctly now!

For sdl-test, it might be nice to get information about where in the source an expression comes from. Gonna try to add that info.

* [.] Add source info to AST

Partially done for now, but needs more work.

In the meantime, trying to fix defn and call not working right.

* [x] Add test (test/inference/defn-and-call.lisp)
* [x] Make repro as small as possible
* [x] Implement test

Types weren't resolved recursively (?t1 to ?t2 to ?t3). Now they are!

For some reason, function move in sdl-test.lisp doesn't get the void type. Need to make a repro for that too. :)

* [x] Add test (test/inference/defn-get-void.lisp)
* [x] Make repro as small as possible
* [x] Implement tests

For some reason, it is unable to resolve the type. Not sure why. D: Maybe the type gets resolved too late or something. A bit unclear...

Lol, I had a special case for `printf` which just didn't create any constraints. Fixed!

Now I have problems with compiling to C, where the cast doesn't end up in the right place.

* [x] Fix defn-and-call so that the cast is right in front of malloc

Now it works! And the game works again!!! :D



---------------



Next up I want to add support for declaring structs.

I don't know how to think about the typing for these.

```
{:x :int
 :y :int}
```

Somehow, they feel similar to variables, just inside something else.

So it kind of feels like the "whole" has one type, then the kvs has their own types. Kind of like ?t and ?t:x ?t:y and so on.

----------------------------------------------

I just saw [Mike Acton's talk on data oriented programming](https://www.youtube.com/watch?v=rX0ItVEVjHc) and thought it was pretty neat. I like the ideas of only solving the problem you have to solve, and move away from abstractions. In this view, Vajer would really be C but with live coding and code generation (macros), and I guess possibly the nicety of type inference for a little bit less typing.

I think it might be nice to write down what I miss from C, that I would like Vajer to have.

* Dynamic lists
  * The question is, how often do I really need a growable list, and how often could I just use a fixed size list?
* Dispatch
  * How often do I actually need dispatch, and how often is it just a way to say "I pretend to not know about the problem"?
  * As long as it is static dispatch, I figure it's more of a nicety for the coder, and it should become pretty fast still
  * In term of dynamic dispatch, I figure it has to do with when I want more generic data structures
* Live code
  * Could this be just as well solved if I could always dump the data properly, and then restart the program
  * Actually not, so maybe live coding is kinda valid
  * Although I need to specify if I want to change functions, add functions, modify structs etc

Niceties
* Structural editing
* Expressions :D
* Pattern matching (??? maybe this is just cursed stuff again)

---

I mean, tbh, I just kinda wanna get on with this thing. I need to be able to interact with structs somehow (if nothing else those I get to/from C). And perhaps "records" or whatever should be another concept. I don't know.

I also realized, that perhaps defining structs should care about the order, i.e. not be a hash map but rather a vector or something. Or it's just a macro with no fancy syntax.

So, yeah. Huh. What now?

I think that I could do the syntax for C-style structs first. Perhaps. And do the records later, if I ever feel like I even need them? But they might be cool!?

Oooooh, that's right, probably even before that, I need to fix nesting / local environments. E.g. code like this:

```
(def a 10)

(defn pront
  [a]
  (printf a)
  )

(defn main
  []
  (pront "hello"))
```

Here `a` in `pront` should be "hello".

Maybe I should fix that first.

* [x] Make a test for shadowing variables

What happens?

It tries to unify `[:char]` with `:int`.

How to fix?

Different type variables in different scopes.

So `defn` must introduce a new scope of some sort.

These scopes are on a stack, so if something isn't on the current scope's environment, it should "go up" one scope and look at that environment, and so on.

Two problems arise:

1. Defining these environments
2. Reading from them

Defining I think is pretty straight forward, whenever we encounter a `defn` we can say "ah, this is a new environment".

But for 2.? I guess it's kinda the same. Whenever we encounter a `defn`, extract the environment from there, or something. Or I guess maybe extraction never needs to happen, it's just a stack while generating type variables. Maybe that's fine.

* [ ] Implement a stack of environments for `assign_type_names` generation

I guess `EnvKV **envs` might work. I should try it separately first.

* [ ] Write a test for a stack of hashmaps

-------------

A different situation:

```
(defn pront1
  [a]
  (printf a)
  )

(defn pront2
  [a]
  (+ a 10)
  )

(defn main
  []
  (pront1 "hello")
  (pront2 3))
```

--------------

I think maybe it shouldn't neccesarily be a stack of environments. I think in the end, the type names should all be "global". So in the same vein, I think the environment should be a big structure. I'm thinking something like this:

```
Environment {
  id: int
  parent: Environment
  children: Environment
  locals: HashMap
}
```

so each Environment can have a parent, and do lookup in the parent (and grandparent etc). But when generating typenames, it just concatenates all ids, like so: "0_10_4_type" which would mean "from environment 0, with child 10, with child 4, that grandchild environment has a type named type".

Okay, so, I did something like this (`experiments/prototype.h`). Now I need to figure out... How to make use of it in the current env-generating code. I think essentially it goes like this:

1. top-level env, `curenv`
2. when encountering a new scope, like a function definition, add a child to `curenv`
3. set `curenv` to that child
4. when encountering a variable binding, put it in `curenv`
5. when exiting the scope, set `curenv` to the parent of `curenv`

Somehow the scope also needs to be associated with the relevant environment. I think just having the "path" (ids needed to go through the children) will work.

---------------------------------------

Now it can handle scoped symbols!!! It turns out that it was easier than I thought. I just had to go through the tree, and rename all scoped symbols from `a` to `a__SCOPE__0` where `0` increases each time a new scoped symbol is introduced. Check `test/inference/same_symbol.lisp` to see what I'm testing (`a` in top level is not same as `a` in `pront`).

I did do some of the prototype-stuff mentioned above. But instead of putting into the type inference stuff (I tried that before...) I instead made it a separate step: `eval.h/ast_add_scopes`.

Nice.

Next up is going back to the problem I wanted to solve in the beginning: defining structs. :)

```
// handle getting struct values
(declare get [?T :symbol => ?T.symbol])

// ...

// handle unifying structs
{x: ?t1, y: ?t2} = {x: int, y: [:char]}
```

Could structs be defined as tuples...? Fields are just offsets?

Some other things I wrote in swedish:

`(declare get [?T :symbol => ?T.symbol])`

`?T` måste inte vara `?T.symbol`, den måste _ha_`?T.symbol`
En struct kommer med ett gäng constraints:
```clojure
(defstruct EnvKV key value)

(def entry (EnvKV "dog" "cat"))

(printf (get entry key))
;;=>
:EnvKV.key = ?t1
:EnvKV.value = ?t2
:EnvKV = {key ?t1, value ?t2}
EnvKV = (?t1 ?t2 => :EnvKV)
entry = :EnvKV
printf = ([:char] => :void)
get = [?t3 :symbol => ?t3.symbol]
(get ?t3 key) = ?t3.key = [:char]
:EnvKV = ?t3
```

Unification måste förstå constraints som `{key ?t1, value ?t2}` och `?t3.symbol`.

Börja med defstruct, låt den introducera en constraint/typ precis som `declare`.



---* [ ] Put `defstruct` in a test---

Hmm, do I even need a `defstruct`...? Maybe... But not now.

* [x] Put `get` in a test

How should the type for `get` really look? Hm.

```
(get {:name "hello} :name)
;;
[?T :keyword => [?T :keyword]]
```

This should lead to a constraint like this:
```
# assuming ?T is ?t
?t = {value-of-kw ?t2}
```


------------------------


Okay so, I think I got pretty far. It seems to type check pretty well.
But now I need to figure out how to, in lisp-land, create structs. I was thinking of something like this:

```
(defstruct Cat {name [:char]})
(var cat (Cat "hello"))
```

But I guess this could work?

```
(defstruct Cat {name [:char]})
(var cat (Cat {name "hello"}))
```

Feels a bit annoying to specify the type, but what to do.













-----------------------------------

TODO: Must ensure nested things work well, like `(if x (var y (insert x y z)))` something. Maybe not that specifically since insert currently returns void.





------------------------------------

Okay, started figuring out how to unify structs. Now I need to figure out how to assign the right type variable to a map that has been identified with a certain struct. :> E.g. accreting multiple `(:has ...)` into a single type, rather than getting only a single `has`-type.

One problem atm is that `(:has ...)` ends up as a type for things, but it should never be a type. :O It should rather end up as a ?t, where ?t is associated with certain constraints... I think maybe the current constraint resolution is too limited to deal with types that can't be determined yet. Hmmm.



-------------------------

Unifying structs works!

--------------------------

* [x] Implement `list`, or something like it...?
  * [x] It should probably be a macro :O

------------------------

I managed to make the macro. It's in `test/macro/list.lisp`.

I realized though, that for macros to be able to call functions, the functions need to be compiled, at least before the macro is called.

After playing around a bit with libtcc, I figured out I can not compile and extract functions using the same tcc-state multiple times. But I am able to compile and extract functions separately, then when the time for calling a macro comes, I can use `tcc_add_symbol` to give a reference to a function. I think this might turn out all right. I did a little bit of this in `experiments/tcc_compile_in_steps.c`.

Next step would be figuring out a nice way to build up this hashmap of C-functions, and to add the declarations to the source of the macro when it is compiled to C.

I think in the beginning I could just add all the functions every time a macro runs.

---* [ ] char *functions_hashmap_to_c(CFunEntry *entries) -- this should take a hashmap with symbol / function+type pairs, and return a string like so:---
```
functions_hashmap_to_c([{test: {f: <void ptr>, type: [:int -> :void]}}])
#=>
"void test();\n"
```
---* [ ] add_tcc_symbols(TCCState *s, CFunEntry *entries) -- does `tcc_add_symbol` for all entries---
* [x] instead I just made a `eval_with_centries`

This might work pretty all right. I also need to get the `CFunEntry *entries` somehow. When compiling a function, it should somehow store the symbol and the type. I guess one way to do that could be to hijack the `c_compile_defun` thing, or I could just loop throgh the (transformed) AST and extract all `defn` calls. I think I only care about top level atm anyway.

---* [ ] AST *find_all_defn(AST *) -- this might work :)---
---* [ ] CFunEntry *defn_to_CFunEntry(AST *defns) -- hm, here I wouldn't have the functions. I guess I need to extract them after compilation is done.---
* [x] `eval` should return `CFunEntry *` :O

* [x] make a partial eval thing where it uses functions from earlier compilations

----------

Okay, so now macros seem to work, when looking in `test/macro/list.lisp`. Pretty cool that a macro can call a function. :)