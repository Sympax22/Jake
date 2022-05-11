# Input registers:
#   a0, a1, a2
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be 0 or 1.
#   If a0 == 0, then set a0 to the value of a1. Else, set a0 to the value of a2.
#   No jumps or branches are allowed except for the return.

# Authorized:
# slliw, srlw, srliw, sraw, sraiw
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

# Hints:
# - None of the authorized instructions permits to change the control flow (e.g., bne and jal are not authorized).

.text
.align	1
.globl	assignment_1_7
.type	assignment_1_7, @function
assignment_1_7:

    # Assignment code.
	slli t1, a0, 31
	sraiw t1, t1, 31
    xori t0, t1, -1
    and t0, a1, t0
    and t1, a2, t1
    or a0, t0, t1
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
