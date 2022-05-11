# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte integer table in memory.
#   Your task is to find the number of nonzero elements before the first zero element in the table.
#   It is guaranteed that the table contains at least one zero element.

# Authorized:
# jal, jalr
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi, sub
# beq, bne, blt, bge, bltu, bgeu

.text
.align	1
.globl	assignment_3_2
.type	assignment_3_2, @function
assignment_3_2:

    # Assignment code.
    addi t0, a0, 0

assignment_3_2_loop:
    lw t1, 0(t0)
    beq t1, x0, assignment_3_2_end
    addi t0, t0, 4
    jal x0, assignment_3_2_loop

assignment_3_2_end:
    sub t0, t0, a0
    srli a0, t0, 2

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
