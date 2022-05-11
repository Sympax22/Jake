# Input register:
#   a0
# Output register: None.

# Task:
#   a0 is the address of a 4-byte integer in memory.
#   Your task is to increment the integer located at this address.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi

.text
.align	1
.globl	assignment_3_0
.type	assignment_3_0, @function
assignment_3_0:

    # Assignment code. almost done

    lw   a1, (a0)           # load the value @a0 in $a1
    addi a1, a1, 1          # increment a1
    sw   a1, (a0)           # store the value a1+1 @a0

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
