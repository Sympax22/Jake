# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   Set a0's value to 1 if a0 == a1, else to zero.
#   Remark: In this assignment, branches are not authorized.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

# Hints:
# - None of the authorized instructions permits to change the control flow (e.g., bne and jal are not authorized).
# - The slt* instructions set the target register to 1 if the result of the comparison is true, else 0.

.text
.align	1
.globl	assignment_1_1
.type	assignment_1_1, @function
assignment_1_1:

    # Assignment code.
    slt t0, a0, a1
    slt t1, a1, a0
    or a0, t0, t1
    xori a0, a0, 1
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
