# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte integer table in memory.
#   Your task is to find the number of nonzero elements before the first zero element in the table.
#   It is guaranteed that the table contains at least one zero element.

# Authorized:
# jal, jalr
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi, sub
# beq, bne, blt, bge, bltu, bgeu

.text
.align	1
.globl	assignment_3_2
.type	assignment_3_2, @function
assignment_3_2:

    # Assignment code. just a couple

    add  a1, zero, zero             # tot_nonz = 0

    loop:                           # while(true):
        lw a2, 0(a0)                # arr[i]
        beq a2, zero, is_zero       # if it's not 0 go on, else return
        addi a1, a1, 1              # it's not zero, so increment tot and the adress
        addi a0, a0, 4  
        beq zero, zero, loop        # jump back to the loop

    is_zero:                        # other case: found 0 element, exit all loops and return in $a0
        add a0, zero, a1


    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
