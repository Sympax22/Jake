# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   The goal of this assignment is to create a recursive function that computes the sum 0+1+...+n. 
#   The base case of the recursion **must** be 0.
#   Because we want to ensure that you are implementing the function recursively and not iteratively, 
#   we will interpose a function f that we provide you through a0.
#   This function f that we provide you takes a single argument k, 
#   and f will (a) register that you called f and (b) call back your function with the 2 arguments a0 and k.
#   a0 is the entry point of a function provided to you, and which will call your function again and check that you call
#   the right number of times with the right parameters.
#   a1 is initially guaranteed to be not larger than 30 and is nonnegative.
#   Your task is to write a recursive sum of 0 to n: returns 0 if a1==0, else return a1+sum(a1-1).
#   For example, for inputs (a0=<address of f>, a1=2), assignment_4_1 will call f with argument a0=1, 
#   then f will call assignment_4_1 with the same argument a1=1 (a0 will again be the address of f), 
#   then assignment_4_1 will call f with argument a0=0, then f will call assignment_4_1 with a1=0, and assignment_4_1 will return the value 0.

# Authorized:
# sll, slli, srl, srli, sra, srai
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# jal, jalr

.text
.align	1
.globl	assignment_4_1
.type	assignment_4_1, @function
assignment_4_1:


    # Assignment code

    sum: 

        # base case
        bne a1, zero, else      # if(k != 0): goto else
        add a0, zero, a1        # k == 0, so load res in $a0
        jr  ra                  # return
        
        # general case
        else:                   # (k != 0)
            add  t0, zero, a0   # save address of f
            addi a0, a1, -1     # k -= 1
            addi sp, sp, -8     # prepare stack
            sw   a1, 4(sp)      # store k on stack
            sw   ra, 0(sp)      # store return address
            jalr t0             # call f
            lw   a1, 4(sp)      # load k back
            add  a0, a0, a1     # res = k + sum(k-1)
            lw   ra, 0(sp)      # load return address
            addi sp, sp, 8      # restore stack

    # -- End of assignment code.
    
    jr ra # Return to the testing framework. Don't modify.
