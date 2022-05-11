# Input registers:
#   a0, a1, a2
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be in {0, 1}.
#   If a0 == 0, then set a0 to the value of a1. Else, set a0 to the value of a2.
#   Arithmetic & logic operations are limited in this assignment, but branches are allowed.

# Authorized:
# add, addi
# beq, bne, blt, bge, bltu, bgeu

.text
.align	1
.globl	assignment_2_0
.type	assignment_2_0, @function
assignment_2_0:

    # Assignment code.

    
    add a7, zero, a0            # a7 = a0
    add a0, zero, a1            # a0 = a1
    beq a7, zero, done          # if (a7 == 0): done
    add a0, zero, a2            # a7 != 0, so a0 = a2

    done:                       # end


    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
