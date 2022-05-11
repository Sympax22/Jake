# Input register:
#   a0,a1
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte (=32-bit) signed integer (tip: we work with 64-bit wide registers).
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

    lw   a2, 0(a0)              # load @a0 into $a2
    beq  a1, zero, done         # if a1 == 0, we have little endian, done
    add  a2, zero, zero         # we have big endian, initialize a2 = 0
    addi a7, zero, 4            # initialize a7 = 4
    addi a4, zero, 8            # initialize a4 = 8
    addi a1, zero, 12           # initialize a1 = 12
    addi a1, a1, 12             # a1 += 12, a1 = 24

    loop:
        beq  a7, zero, done     # for i in range(4)
        lbu  a3, 0(a0)          # load the first byte of reg @a0 into a3
        sll  a3, a3, a1         # shift by a1
        add  a2, a2, a3         # a2 += a3
        addi a7, a7, -1         # i--
        addi a0, a0, 1          # next byte
        sub  a1, a1, a4         # a1 -= 4
        beq  zero, zero, loop   # goto loop

    done:
        add a0, a2, zero        # store value in a0

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
