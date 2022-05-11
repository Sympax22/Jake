# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   Invert the a0 value bitwise (each zero bit of a0 must be set to 1 and vice-versa).

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_4
.type	assignment_1_4, @function
assignment_1_4:

    # Assignment code.
	xori a0, a0, -1
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
