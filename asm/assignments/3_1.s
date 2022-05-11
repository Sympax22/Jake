# Input registers:
#   a0, a1
# Output register: None.

# Task:
#   a0 is the base address of a 4-byte integer table in memory.
#   Your task is to increment the a1-th element of the table.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# sll, slli, srl, srli, sra, srai

.text
.align	1
.globl	assignment_3_1
.type	assignment_3_1, @function
assignment_3_1:

    # Assignment code.

    slli a1,  a1, 2         # a1 *= 4
    add  a2,  a1, a0        # address is @ a0 + 4*a1
    lw   a3, 0(a2)          # load the value @ a0 + 4*a1 into $a3
    addi a3,  a3, 1         # increment
    sw   a3, 0(a2)          # store a3+1 @ a0 + 4*a1

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
