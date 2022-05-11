# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is the entry point of a function provided to you.
#   Your task is to transfer control to this function.
#   Don't forget to provide the return address in x1 & to save it beforehand.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# jal, jalr

.text
.align	1
.globl	assignment_4_0
.type	assignment_4_0, @function
assignment_4_0:

    # Assignment code.

    addi  sp, sp, -4            # prepare stack
    sw    ra, 0(sp)             # store return address
    jalr  ra, a0                    # call function, addr @a0+0
    lw    ra, 0(sp)             # load return address
    addi  sp, sp, 4             # restore stack

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
