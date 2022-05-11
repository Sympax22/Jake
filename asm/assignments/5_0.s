# Input registers:
#   a0, a1, a2, a3, a4, a5
# Output register: None

# Task:
#   Your task is to multiply two matrices M0 and M1 of 2-byte signed integers and to store the resulting matrix M2 in memory.
#   a0 contains the base address of M0.
#   a1 contains the width of M0  (number of columns). Included in [1, 30]. It is the same as the height of M1.
#   a2 contains the height of M0 (number of rows). Included in [1, 30]. It is the same as the height of M2.
#   a3 contains the base address of M1.
#   a4 contains the width of M1  (number of columns). Included in [1, 30]. It is the same as the width of M2.
#   a5 contains the (pre-allocated) address where to store M2.

# Authorized:
# sll, slli, srl, srli, sra, srai
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# j, jal, jalr
# sll, slli, srl, srli, sra, srai
# sub, lui
# xor, xori, or, ori, and, andi
# slt, slti, sltu, sltiu
# mul

# Hints:
# - Feel free to modify the file tester_main.c, for example to add prints. This may help you a lot debugging your code.

.text
.align	1
.globl	assignment_5_0
.type	assignment_5_0, @function
assignment_5_0:

    # Assignment code.
# t0: curr target x
# t1: curr target y
# t2: curr x in M0 (and curr y in M1)
# t3: curr cumulated value
# t4: load address tmp
# t5: load address tmp
    # Reset target y coordinate.
    add t1, x0, x0
assignment_5_0_height_loop:
    # Check whether we reached the height of M0.
    beq t1, a2, assignment_5_0_finally
    add t0, x0, x0
assignment_5_0_width_loop:
    # Check whether we reached the width of M1.
    bne t0, a4, assignment_5_0_domul
    # If we reached the end of the row, then reset x to 0 and increment y.
    add t0, x0, x0
    addi t1, t1, 1
    j assignment_5_0_height_loop

# Start the multiplication for the target coefficient x,y.
assignment_5_0_domul:
    # Unset the relevant registers.
    add t2, x0, x0
    add t3, x0, x0

assignment_5_0_domul_loop:
    # Check whether we reached the width of M0 (equal height of M1).
    beq t2, a1, assignment_5_0_domul_complete
    # Compute the load address in M0 (each word has 8 bytes), and then load.
    mul t4, a1, t1
    add t4, t4, t2
    slli t4, t4, 1
    add t4, t4, a0
    lh t4, (t4)
    # Do the same in M1.
    mul t5, a4, t2
    add t5, t5, t0
    slli t5, t5, 1
    add t5, t5, a3
    lh t5, (t5)
    # Multiply the two coefficients together and cumulate them.
    mul t4, t4, t5
    add t3, t3, t4
    # Advance
    addi t2, t2, 1
    j assignment_5_0_domul_loop

assignment_5_0_domul_complete:
    # Store the cumulated value at the right address.
    mul t4, t1, a4
    add t4, t4, t0
    slli t4, t4, 1
    add t4, t4, a5
    sh t3, (t4)

    # Go to the next target coefficient.
    addi t0, t0, 1
    j assignment_5_0_width_loop

assignment_5_0_finally:
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
