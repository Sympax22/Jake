# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be in [0, 48]
#   Compute the a0-th term of the Fibonacci sequence, then store it into a0.

# Authorized:
# beq, bne, blt, bge, bltu, bgeu
# add, addi, sub
# jal

.text
.align	1
.globl	assignment_2_1
.type	assignment_2_1, @function
assignment_2_1:

    # Assignment code. this sucks

    
    beq  a0, zero, is_zero          # if (n == 0), branch
    addi a1, zero, 0                # all other cases, f1 stored in $a1
    addi a2, zero, 1                # f2 stored in $a2
    addi a3, zero, 0                # temp stored in a3
    addi a7, zero, 0                # loop variable i stored in $a7

    loop:                           # for loop --- for i in range(n):
        beq  a7,   a0, exit_loop
        add  a3, zero, a2           # temp = f2     
        add  a2,   a2, a1           # f2 = f2 + f1
        add  a1, zero, a3           # f1 = temp
        addi a7,   a7, 1            # i++
        beq zero, zero, loop        # j loop

    # --- End of loop ---    

    exit_loop:                      # done counting, return f1
        add   a0, zero, a1
        beq zero, zero, done

    is_zero:                        # base case
        add   a0, zero, zero

    done:                           # end

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
