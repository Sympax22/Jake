# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   Set a0's value to 0xbadcab1e.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_3
.type	assignment_1_3, @function
assignment_1_3:

    # Assignment code.

    lui  a0, 0x7ADCA  # load the first 20 bits in [31:11], subtracting enough to get leading 0 and not 1 
                      # (64-bit register with 2's complement):            0x000000007ADCA000
    lui  a1, 0x40000  # we lack 0x4000b1e, so we load it to a second reg: 0x0000000040000000
    add  a0, a0, a1   # add the two:                                      0x00000000BADCA000
    srli a0, a0, 4    # prepare to add 0xB1E, shift first:                0x000000000BADCA00
    addi a0, a0, 0xB1 # add 0xB1:                                         0x000000000BADCAB1
    slli a0, a0, 4    # shift:                                            0x00000000BADCAB10
    addi a0, a0, 0xE  # add 0xE:                                          0x00000000BADCAB1E

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
