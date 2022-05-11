# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   The goal of this assignment is to create a recursive function that computes the sum 0+1+...+n. The base case of the recursion **must** be 0.
#   Because we want to ensure that you are implementing the function recursively and not recursively, we will interpose a function f that we provide you through a0.
#   This function f that we provide you takes a single argument k, and f will (a) register that you called f and (b) call back your function with the 2 arguments a0 and k.
#   a0 is the entry point of a function provided to you, and which will call your function again and check that you call the right number of times with the right parameters.
#   a1 is initially guaranteed to be not larger than 30 and is nonnegative.
#   Your task is to write a recursive sum of 0 to n: returns 0 if a1==0, else return a1+sum(a1-1).
#   For example, for inputs (a0=<address of f>, a1=2), assignment_4_1 will call f with argument a0=1, then f will call assignment_4_1 with the same argument a1=1 (a0 will again be the address of f), then assignment_4_1 will call f with argument a0=0, then f will call assignment_4_1 with a1=0, and assignment_4_1 will return the value 0.

# Authorized:
# sll, slli, srl, srli, sra, srai
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# jal, jalr

.text
.align	1
.globl	assignment_4_1
.type	assignment_4_1, @function
assignment_4_1:

    # Assignment code.
    addi sp, sp, -16
    sd   ra, 0(sp)
    sd   s1, 8(sp) # Will store a0

    # Base case: when a1 = 0
    beq a1, x0, assignment_4_1_base_case

    # Store a1 (i.e., n) temporarily because it will be overwritten.
    addi s1, a1, 0

    # Save a0
    addi t0, a0, 0

    # Recursive call.
    addi a0, a1, -1
    jalr ra, t0

    # Now s0 contains the recursive call address, a0 the sum 1...(n-1) and a1 contains an unspecified value.
    # s1 contains n.
    add a0, a0, s1

    j assignment_4_1_finally
assignment_4_1_base_case:
    addi a0, x0, 0 # In the base case, return 0
assignment_4_1_finally:
    ld   ra, 0(sp)
    ld   s1, 8(sp)
    addi sp, sp, 16

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
