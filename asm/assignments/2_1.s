# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be in [0, 48]
#   Compute the a0-th term of the Fibonacci sequence, then store it into a0.

# Authorized:
# beq, bne, blt, bge, bltu, bgeu
# add, addi, sub
# jal

.text
.align	1
.globl	assignment_2_1
.type	assignment_2_1, @function
assignment_2_1:

    # Assignment code.

    # Return if a0 is zero. 
    beq a0, x0, assignment_2_1_zero

    # # Offset the index in the sequence.
    addi a0, a0, -1

    # Initialize the first two terms.
    addi t0, x0, 0
    addi t1, x0, 1

    # Constant equal to 1.
    addi t3, x0, 1
assignment_2_1_loop:
    blt a0, t3, assignment_2_1_branch_tgt

    # Iterate the addition, with one buffer element.
    add t2, t1, t0
    add t0, x0, t1
    add t1, x0, t2
    # Decrement a0.
    addi a0, a0, -1
    jal x0, assignment_2_1_loop

assignment_2_1_zero:
    addi t1, x0, 0
assignment_2_1_branch_tgt:
    addi a0, t1, 0
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
