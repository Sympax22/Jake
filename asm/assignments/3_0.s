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

    # Assignment code.
    lw t0, 0(a0)
    addi t0, t0, 1
    sw t0, 0(a0)
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
