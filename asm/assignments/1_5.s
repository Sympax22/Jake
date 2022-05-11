# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer. It is guaranteed to be included in [1, 32].
#   Your task is to create a binary number with a0 ones on the most significant bit side, and the potential other bits are zero.
#   Example: if a0 = 9, then a0 must be set to 0b11111111100000000000000000000000 

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_5
.type	assignment_1_5, @function
assignment_1_5:

    # Assignment code.

    addi a7, zero,   -1         # load 32 zeros in $a7    
    addi a6, zero, 0x20         # load 32 in $a6
    sub  a6,   a6,   a0         # compute 32 - a0 in $a6
    srl  a7,   a7,   a6         # we now know how many zeros we want, and can shift by a0 to the right, and then back to left
    sll  a7,   a7,   a6         # we shift back
    add  a0, zero,   a7         # store all in $a0

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
