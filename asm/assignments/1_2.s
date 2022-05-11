# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   Set a0's value to 1 if a0 is negative (0 is considered positive), else to zero.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_2
.type	assignment_1_2, @function
assignment_1_2:

    # Assignment code.
    srli a0, a0, 31
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
