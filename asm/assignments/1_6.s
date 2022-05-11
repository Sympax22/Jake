# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   a1 is guaranteed to be included in [1, 32].
#   Your task is to zero out the a1 top bits of a0.
#   Example: if a0 = 0b11010001101000100111001110001101 and a1 = 7, then a0 must be set to 0b00000001101000100111001110001101 

# Authorized:
# slliw, srlw, srliw, sraw, sraiw
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_6
.type	assignment_1_6, @function
assignment_1_6:

    # Assignment code.
	addi t0, x0, 1 # Set t0 to constant 1
	slli t1, t0, 31
    sub  t2, a1, t0
    sraw a1, t1, t2
    xori a1, a1, -1
    and  a0, a0, a1
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
