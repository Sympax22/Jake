# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   Set a0's value to 0xbadcab1e.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

# Hints:
# - The auipc instruction sets the 20 top bits of the target register.
# - The addi instruction sets the 12 bottom bits of the target register. But problem: it sign-extends it! Can you work around it?

.text
.align	1
.globl	assignment_1_3
.type	assignment_1_3, @function
assignment_1_3:

    # Assignment code.
   lui	a0,0x5d6e5
   addi	a0,a0,1423
   c.slli	a0,0x1
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
