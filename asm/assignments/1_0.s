# This is an example assignment that computes the sum of two registers.
# You are not supposed to modify this file. It is not graded.

# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   Compute the sum of the input registers.

# Authorized:
# add

.text
.align	1
.globl	assignment_1_0
.type	assignment_1_0, @function
assignment_1_0:

    # Assignment code.
    add a0, a1, a0
    # -- End of assignment code.
    jr ra # Return to the testing framework. Don't modify.

