# Input registers:
#   a0, a1, a2, a3, a4, a5
# Output register: None

# Task:
#   Your task is to multiply two matrices M0 and M1 of 
#   2-byte signed integers and to store the resulting matrix 
#   M2 in memory.
#   a0 contains the base address of M0.
#   a1 contains the width of M0  (number of columns). 
#   Included in [1, 30]. It is the same as the height of M1.
#   a2 contains the height of M0 (number of rows). Included in [1, 30]. 
#   It is the same as the height of M2.
#   a3 contains the base address of M1.
#   a4 contains the width of M1  (number of columns). 
#   Included in [1, 30]. It is the same as the width of M2.
#   a5 contains the (pre-allocated) address where to store M2.

# Authorized:
# sll, slli, srl, srli, sra, srai
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# j, jal, jalr
# sll, slli, srl, srli, sra, srai
# sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu
# mul

.text
.align	1
.globl	assignment_5_0
.type	assignment_5_0, @function
assignment_5_0:

    # Assignment code.
    add t0, zero, zero
    add t3, a0, zero
    row_loop:

        add t1, zero, zero
        add t4, a3, zero
        col_loop:

            add t2, zero, zero  # counter for element loop
            add t5, t3, zero    # adress of element in M0
            add s10, t4, zero    # adress of element in M1
            add s9, zero, zero  # initialize s9
            element:
                lh a6, 0(t5)    # load the value of M0 in $a6
                lh a7, 0(s10)   # load the value of M1 in $a7
                mul s11, a6, a7 # multiply and store in $s11
                add s9, s11, s9 # add to subresult
                addi t5, t5, 2   # increase address of M0 element
                slli a4, a4, 1  # increase address of M1 element
                add s10, s10, a4
                srli a4, a4, 1
                addi t2, t2, 1
            bne t2, a1, element # if element calculated, leave
            add zero, zero, zero
            
            sw s9, 0(a5)        # save element of M2
            addi a5, a5, 2      # move to next element of M2
            addi t4, t4, 2      # move to next column of M1
            addi t1, t1, 1      # increase column counter 
        bne t1, a4, col_loop    # if all columns of M1 finished, leave loop
        add zero, zero, zero
        slli a1, a1, 1          # double number of columns of M0
        add t3, t3, a1          # move to next row in M0
        srli a1, a1, 1          # half number of columns of M0
        addi t0, t0, 1          # increase row counter
    bne t0, a2, row_loop        # if all rows of M0 finished, leave loop
    add zero, zero, zero

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
