# Input registers:
#   a0, a1, a2
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be 0 or 1.
#   If a0 == 0, then set a0 to the value of a1. Else, set a0 to the value of a2.
#   No jumps or branches are allowed except for the return.

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_7
.type	assignment_1_7, @function
assignment_1_7:

    # Assignment code.
    
    add   a5,   a0,  zero           # store a0 and 1-a0, a5 = a0
    addi  a4, zero,  0x01
    sub   a4,   a4,    a0           # a4 = 1 - a0
    slli  a7,   a5,  5              # shift by 5 and store in different regs, a7 = a0 * 32
    slli  a6,   a4,  5              # a6 = (1 - a0) * 32
    sub   a7,   a7, a5              # we can only shift by 31 max at once, a7 = a0 * 31
    sub   a6,   a6, a4              # a6 = (1 - a0) * 31 

    # Rule of thumb: if a0 = 0, don't touch $a1

    srl   a2,   a2,   a6            # shift a2 by 31 if a0 = 0, else shift by 0
    srl   a1,   a1,   a7            # shift a1 by 31 if a0 = 1, else shift by 0  
    srl   a1,   a1,   a5            # shift a1 by 0 if a0 = 0, else by 1
    srl   a2,   a2,   a4            # shift a2 by 0 if a0 = 1, else by 1

    # final shift: 31 + 1 and else 0 + 0

    add   a0,   zero,   a2          # a0 is 1: a0 becomes a2 (if it's not, we now have a0 = 0)    
    add   a0,     a0,   a1          # a0 is 0: a0 becomes a1 (if it's not, we added 0)

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
