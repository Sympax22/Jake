# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   Set a0's value to 1 if a0 == a1, else to zero.
#   Remark: In this assignment, branches are not authorized.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_1
.type	assignment_1_1, @function
assignment_1_1:

    # Assignment code

    sub  a0,   a0, a1         # a0 -= a1
    sltu a0, zero, a0         # if 0 < |diff|, then a0 = 1
    xori a0,   a0, 1          # invert result

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
