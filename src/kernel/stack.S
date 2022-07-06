# stack.S: First thing that runs---on all CPUs at the same time! No need to
# pick one for initialization purposes

# Why does this work without setting MIE or SIE?
# 	1. Once we're in S-mode, privilege mode is less than M ->
# 	2. M-mode interrupts are globally enabled (p. 31) ->
# 	3. medeleg to scall (p. 30)
#
# Please note that the ISA specifies that we should always support handling
# exceptions in M-mode, but we currently don't

.extern main # .extern is ignored by the assembler, but not by humans
.extern scall
.extern mcall
.extern timer_setup
.extern trap_handler_s
.extern trap_handler
.extern tf_generic
.extern tf_temp
# FIXME: there's also a TOP_STACK in buddy.c, make sure there's only one .globl?
.equ TOP_STACK, 0x20000 # 512 MB  
# 10 pages for stack, means the lowest is 0x1fff6 (? 0xa000 ?)
# we assign 2 pages for mmachine, 0x1fff8
   
.section .text
_entry:
	# Initialize stack pointer
	auipc sp, TOP_STACK
	# Disable paging
	csrw satp, 0
	# Return to S-mode
	xor t0, t0, t0
	xor t0, t0, 0x01
	sll t0, t0, 0xb
	# Optionally setting MIE and SIE in mstatus
	# xor t0, t0, 0xa
	#li t0, 0xA # 0x8 + 0x2
	csrw mstatus, t0
	
	# Set interrupt vector in S-mode to scall
	la t0, trap_handler_s
	csrw stvec, t0
	# Set interrupt vector in M-mode to mcall (just in case)
	la t0, trap_handler
	csrw mtvec, t0
		
	li t0, 0xB9FF
	csrw medeleg, t0
	li t0, 0xFFFF
	csrw mideleg, t0

	li t1, 0x9fff7
	slli t1, t1, 12
	csrw mscratch, t1	

	# Set return address for mret to main
	la t0, main
	csrw mepc, t0
	mret
