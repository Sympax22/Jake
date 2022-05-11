% GENERAL(2) COMSEC | Computer Engineering '22: Homework Guide

# NAME
general - General instructions for the kernel homework assignments: `buddy(3)`, `paging(4)`, `user(6)`, and `multiprocessing(7)`

# NOTE

The instructions for the assignments `asm(1)` and `caching(5)` are given on their respective man pages

# DESCRIPTION
## Handing in

Use `make tar` (see below) to create `jake.tar.gz`. Upload `jake.tar.gz` on Moodle.
 
## Reading our code

Our coding style is inspired by the coding style of the Linux kernel, which you will find easier to read after having taken this course. However, we will definitely not impose a particular style on you (see "Reading your code" below). In the end, coding style is a matter of taste. The list below is simply our preference, but there exist very good alternatives as well. These guidelines are here just to help you read our code, you may also completely ignore them.

* Macros are great, but don't forget the outer parentheses
* Structures are great, but please don't typedef them unless you have the intention to make them opaque
* Give meaningful functions names, most functions will either do something, such as `buddy_remove`, or check something, such as `is_list_empty`
* Also give meaningful variable names, although this is less important. For variables, you sometimes just want them to be short, so make use of common abbreviations such as `i`, `tmp`, `old`, `new`, `prev`, `next`, etc.
* If a function checks something, return 0 if false and 1 if true
* If a function does something, either
	1. return a pointer to whatever was asked for (such as a block for `buddy_remove`) and a `NULL` pointer upon failure; or, alternatively
	2. return an error code of type `int`, with 0 being success and `-ERROR` being an error, and optionally (ab)use an argument as a return value. Using an argument as a return value can be tricky, but also very useful
* Functions should be short and mostly self-explanatory. Make them do just one thing. What does short mean? Not too many lines of code (<= 50), not too many levels of indentation (<= 2), not too many arguments (<= 2 except if you're initializing something), and not too many local variables (<= 5). But be flexible, these are just guidelines in the end
* Use `__` in front of functions (and sometimes variables) to indicate that this function does "the real work" which has two meanings:
	1. It is called by another function, usually with the same name but without the `__`, that does some form or argument validation and/or administrative tasks before passing on the arguments to its `__` counterpart
	2. It assumes that the arguments make sense and is therefore not always safe to call. In other words, `__` also acts as a warning saying "this function makes nontrivial assumptions" or "this function is easily misunderstood"
* Related to the above: it is perfectly fine to use `gotos` in a function that does argument validation and/or administrative tasks

## Reading your code

We encourage short and sweet code that contains lots of small functions, but will not penalize badly written code that works. However, if it is obvious that you have tried to write code that just passes our tests, instead of code that does what we asked you to make it do, we might lower your grade.

## Using our code or writing your own

If you don't like our code and would rather implement everything from scratch, feel free to do so. Or if you don't like one particular function we gave you, go ahead and rewrite it. (And ideally let us know why yours is better, so that we may also learn.) However, please don't change

* the interfaces (i.e., anything in a `*.h` file), as that might break our tests
* the tests themselves
* the `Makefile`

We will also not help you debugging the code you change outside of the TODOs. Unfortunately, there's simply no time for that.

## Makefile 

Here's a brief explanation of some useful `make` targets:

### make

If you issue `make` without specifying a target, it will process the first rule, which is `grade`. In other words, `make` is equivalent to `make grade`. You will be using this command most of the time

### make grade

An alias for `make test qemu` (see below)

### make jake

Compiles the kernel, without tests, gives you `jake`

### make test

Compiles the kernel, but with tests, also gives you `jake`

### make qemu

Uses QEMU to virtualize `jake`. Assumes `jake` exists

### make dis

Disassembles `jake` using `objdump`. Assumes it exists

### make gdb

Attaches `gdb` to `jake`. Assumes it exists

### make tar

Prepares your work for submission by archiving `./src/kernel/`. Does a `make clean` (see below) before creating the archive

### make clean

Removes all object files, including `jake`

## Debugging

Debugging a kernel is hard, in particular because there is nothing to fall back on if things don't work. As a consequence, your kernel will crash silently most of the time, leaving it entirely up to you to find out what went wrong.

To make matters worse, our tests are compiled into your kernel, which means that you might also make our tests crash. Needless to say, if a test crashes it will not be able to give you nice feedback on why it failed. This is completely normal. You will have to get used to it. :)

There's also some good news, however. There are some debugging tools that will almost always be at your disposal:

- The `printdbg` function (or printing in general) allows you to quickly print a variable. Will work most of the time
- The GNU Debugger or `gdb(1)` will always work and allows you to step through your program as it runs. Requires some practice, can be time-consuming, but will give you a lot of information
- `objdump(1)` will try to disassemble your kernel and show you the assembly instructions it consists of, which may sometimes be useful as well (you probably want to use `make dis` for this)

## Questions

If you have a question that you think might be relevant for other students as well, use Moodle. Otherwise, please ask us during the office hours or the tutorials.

# TIPS

- Use `clang-format(1)` to automatically format your code
- Make use of the helper functions in `util.h` and feel free to add new ones yourself
- You should not find yourself writing boilerplate code. The lines that we do ask of you, although not many, are the most fun and difficult ones

# AUTHORS
Finn de Ridder
