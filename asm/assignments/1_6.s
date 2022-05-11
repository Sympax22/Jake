# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   a1 is guaranteed to be included in [1, 32].
#   Your task is to zero out the a1 top bits of a0.
#   Example: if a0 = 0b 1101 0001 1010 0010 0111 0011 1000 1101 and a1 = 7, then a0 must be set to 0b00000001101000100111001110001101 

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_6
.type	assignment_1_6, @function
assignment_1_6:

    # Assignment code.

    addi a7, zero, -1           # set up the ones-string in $a7
    srl  a7,   a7, a1           # shift $a7 by a1 to the right
    srli a7,   a7, 32           # shift by 32 to have 0 on the right for the last 4 bytes
    and  a0,   a0, a7           # a0 = a0 && a7

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
