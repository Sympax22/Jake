# Input register:
#   a0,a1
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte signed integer (tip: we work with 64-bit wide registers).
#   a1 indicates whether this integer has been stored in little-endian (a1 = 0) or big-endian (a1 = 1) mode.
#   Your task is to retrieve the integer from memory and to return it.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# add, addi, sub, or
# sll, slli, srl, srli, sra, srai
# beq, bne
# jal, jalr

.text
.align	1
.globl	assignment_3_3
.type	assignment_3_3, @function
assignment_3_3:

    # Assignment code.
    bne x0,a1,assignment_3_3_big_endian
    # Little endian.
    lw a0,(a0)
    jal x0,assignment_3_3_finally

assignment_3_3_big_endian:
    # Loading zero-extends (LBU) or sign-extends (LB) to 32 bits.
    lb  t0,0(a0) # MSB, load as signed for sign extension.
    lbu t1,1(a0)
    lbu t2,2(a0)
    lbu t3,3(a0) # LSB.

    slli t0,t0,24
    slli t1,t1,16
    slli t2,t2,8

    add a0,x0,x0
    or a0, a0, t0
    or a0, a0, t1
    or a0, a0, t2
    or a0, a0, t3

assignment_3_3_finally:
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
