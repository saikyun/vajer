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