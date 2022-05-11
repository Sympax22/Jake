# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer. It is guaranteed to be included in [1, 32].
#   Your task is to create a binary number with a0 ones on the most significant bit side, and the potential other bits are zero.
#   Example: if a0 = 9, then a0 must be set to 0b11111111100000000000000000000000 (ignoring the 32 top bits, which are zero because a0 is 32 bits long.

# Authorized:
# slliw, srlw, srliw, sraw, sraiw
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_5
.type	assignment_1_5, @function
assignment_1_5:

    # Assignment code.
	addi t0, x0, 1 # Set t0 to constant 1
	slli t1, t0, 31
    sub  t2, a0, t0
    sraw  a0, t1, t2
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
