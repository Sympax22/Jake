% ASM(1) COMSEC | Computer Engineering '22: Homework Guide
% Flavien Solt

# NAME
asm - Introduction to RISC-V assembly

# DEADLINE
Friday, March 11, 2022 @ 11:59 (noon)

# SOURCES
./asm

# HANDING IN
Generate the `handout.tar.gz` file using the `make zip` command when in the asm directory.
Then upload `handout.tar.gz` on Moodle.

# COMMANDS
## make [grade]
Runs the tests

## make details
Gives more details about your grade

## make zip
Generates the `handout.tar.gz` file

# DESCRIPTION

## RISC-V assembly in short

Programmers like to type code in high-level languages such as C and C++, but a CPU is unable to understand these languages directly. _Compilation_ is the action of transforming sources written in a higher-level language into a representation understandable by a CPU: the _bytecode_.

Because bytecode is simply a sequence of bytes, it is very difficult to read for humans. _Assembly language_ is therefore a low-level language, readable by humans, which describes sequences of instructions that map directly to bytecode.

For example, the following snippet of C codes is compiled in the following way:

C source:
```c
a = a+b
```

Corresponding RISC-V assembly (assuming `a` is stored in register `a5` and `b` is stored in register `a4`):
```asm
addw    a5,a5,a4
```

## Instruction set architectures
Historically, different vendors built CPUs that support various types of instructions.
The set of instructions accepted by a CPU, along with the binary representation of each instruction, is called an _instruction set architecture_ (ISA).

Some ISAs only contain a modest number of instructions (for example RISC-V or MIPS) and are called reduced instruction set computers (RISC). Some others contain a much larger number of instructions, typically more than a thousand. This is the case of the wide-spread x86 ISA.

## RISC-V
RISC-V is a recent free ISA, whereas typical ISA vendors require CPU manufacturers to pay a fee to build CPUs that understand their ISA.

The RISC-V ISA is modular: the base ISA RISCV32I has a very small number of instructions.
This base ISA can then be augmented with various extensions that add instructions supporting features such as floating-point operations, compressed instructions or multiplications and divisions.

# TASKS
During this session, you are asked to implement relatively small snippets of code in RISC-V assembly, in files with the `.s` extension located in the `assignments` directory.

We encourage you to use a RISC-V cheat sheet to have an overview of the RISC-V instructions, for example [this one](https://www.cl.cam.ac.uk/teaching/1516/ECAD+Arch/files/docs/RISCVGreenCardv8-20151013.pdf).

## Forbidden instructions

A comment in each assignment indicates which instructions you are allowed to use. All instructions and pseudo-instructions are forbidden if not explicitly authorized.

# TIPS

Be careful to preserve the callee-saved registers and to respect the general RISC-V call conventions (for instance, function arguments are provided in registers a0,a1,... and return values are generally provided in register a0) (more information [here](https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf)).

Be careful to not overwrite the stack written by functions calling the current function (in the more advanced snippets).
The stack pointer is always located at the last already-populated double-word.

## Debugging

We provide you part of the code (`tester_src/tester_main.c`) that checks your answers.
You may modify it to help you debugging.
However, you may sometimes want to use the original version of this file to make sure that your results are correct.

If you do not run using `make details`, then you can read the output of you run in `build/exec_out.txt`.

## Infinite loops and segmentation faults

Because your code will be run as part of the testing framework, your code may cause bad behavior of the test framework leading to a test looping forever, or the test system crashing.
In any of these cases, you will receive a total grade of 0.

To help you debug, here are the most common causes of such bad behavior:

- Your code loops infinitely
- Your code overwrites the stack
- Your code overwrites some callee-saved registers without saving and restoring them

# TESTS

This session is made of 1 example assignment and 15 small assignments of respective grading scale:

| Assignment | Grading scale | Difficulty (indicative) |
|------------|---------------|-------------------------|
| 1.0        |  0 (example)  |                         |
| 1.1        |  5            | easy                    |
| 1.2        |  5            | easy                    |
| 1.3        |  5            | easy                    |
| 1.4        |  5            | medium                  |
| 1.5        |  5            | medium                  |
| 1.6        |  5            | medium                  |
| 1.7        |  5            | medium                  |
| 2.0        |  7            | medium                  |
| 2.1        |  7            | difficult               |
| 3.0        |  7            | easy                    |
| 3.1        |  7            | easy                    |
| 3.2        |  7            | easy                    |
| 3.3        |  7            | medium                  |
| 4.0        |  7            | easy                    |
| 4.1        |  7            | medium                  |
| 5.0        |  9            | difficult               |
| Total      | 100           |                         |

Each of these assignments is graded as pass or fail.

The assignment 1.0 is to provide you with an example. It is not graded and you are not expected to modify it.

# COLOPHON
This is the first homework assignment of Computer Engineering '22.
