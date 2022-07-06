.globl trap_handler
.globl trap_handler_s

.section .text
.align 4 // FIXME: is this needed?
trap_handler_s:
	csrrw sp, sscratch, sp  // sscratch = sp_u  
    addi sp, sp, -264
    
	sd ra, 8(sp)
	sd t0, 16(sp)
	sd t1, 24(sp)
	sd t2, 32(sp)
	sd a0, 40(sp)
	sd a1, 48(sp)
	sd a2, 56(sp)
	sd a3, 64(sp)
	sd a4, 72(sp)
	sd a5, 80(sp)
	sd a6, 88(sp)
	sd a7, 96(sp)
	sd t3, 104(sp)
	sd t4, 112(sp)
	sd t5, 120(sp)
	sd t6, 128(sp)

	csrr t0, satp
	sd t0, 136(sp)

	csrr t0, sepc
	sd t0, 144(sp)

	sd gp,  152(sp)
	sd tp,  160(sp)
	sd s0,  168(sp)
	sd s1,  176(sp)
	sd s2,  184(sp)
	sd s3,  192(sp)
	sd s4,  200(sp)
	sd s5,  208(sp)
	sd s6,  216(sp)
	sd s7,  224(sp)
	sd s8,  232(sp)
	sd s9,  240(sp)
	sd s10,	248(sp)
	sd s11,	256(sp)

    // Save user sp
    csrrw t0, sscratch, t0  
    sd t0, 0(sp)

    // Restore original sscratch 
    add t0, x0, sp
    addi t0, t0, 264
    csrw sscratch, t0

	call scall

    ld ra, 8(sp)
	ld t0, 16(sp)
	ld t1, 24(sp)
	ld t2, 32(sp)
	ld a0, 40(sp)
	ld a1, 48(sp)
	ld a2, 56(sp)
	ld a3, 64(sp)
	ld a4, 72(sp)
	ld a5, 80(sp)
	ld a6, 88(sp)
	ld a7, 96(sp)
	ld t3, 104(sp)
	ld t4, 112(sp)
	ld t5, 120(sp)
	ld t6, 128(sp)

	ld s0, 136(sp)
	csrw satp, s0
	
	# DO NOT LOAD SEPC!
	# sepc is changed in the functions!

	ld gp,  152(sp)
	ld tp,  160(sp)
	ld s0,  168(sp)
	ld s1,  176(sp)
	ld s2,  184(sp)
	ld s3,  192(sp)
	ld s4,  200(sp)
	ld s5,  208(sp)
	ld s6,  216(sp)
	ld s7,  224(sp)
	ld s8,  232(sp)
	ld s9,  240(sp)
	ld s10,	248(sp)
	ld s11,	256(sp)
    ld sp, 0(sp)

	sret

.align 4
trap_handler:
	csrrw sp, mscratch, sp  // mscratch = sp_u  
    addi sp, sp, -264
    
	sd ra, 8(sp)
	sd t0, 16(sp)
	sd t1, 24(sp)
	sd t2, 32(sp)
	sd a0, 40(sp)
	sd a1, 48(sp)
	sd a2, 56(sp)
	sd a3, 64(sp)
	sd a4, 72(sp)
	sd a5, 80(sp)
	sd a6, 88(sp)
	sd a7, 96(sp)
	sd t3, 104(sp)
	sd t4, 112(sp)
	sd t5, 120(sp)
	sd t6, 128(sp)
	
	csrr t0, satp
	sd t0, 136(sp)

	csrr t0, mepc
	sd t0, 144(sp)

	sd gp,  152(sp)
	sd tp,  160(sp)
	sd s0,  168(sp)
	sd s1,  176(sp)
	sd s2,  184(sp)
	sd s3,  192(sp)
	sd s4,  200(sp)
	sd s5,  208(sp)
	sd s6,  216(sp)
	sd s7,  224(sp)
	sd s8,  232(sp)
	sd s9,  240(sp)
	sd s10,	248(sp)
	sd s11,	256(sp)

    // Save user sp
    csrrw t0, mscratch, t0  // t0 = sp_u. mscratch = t0_u
    sd t0, 0(sp)

	add a0, x0, sp		// argument to mcall

    // Restore original mscratch 
    add t0, x0, sp
    addi t0, t0, 264
    csrw mscratch, t0

	call mcall

    ld ra, 8(sp)
	ld t0, 16(sp)
	ld t1, 24(sp)
	ld t2, 32(sp)
	ld a0, 40(sp)
	ld a1, 48(sp)
	ld a2, 56(sp)
	ld a3, 64(sp)
	ld a4, 72(sp)
	ld a5, 80(sp)
	ld a6, 88(sp)
	ld a7, 96(sp)
	ld t3, 104(sp)
	ld t4, 112(sp)
	ld t5, 120(sp)
	ld t6, 128(sp)

	ld s0, 136(sp)
	csrw satp, s0
	
	# we could change mepc,
	# but it is kept symmetric to scall() handler
	# ld s0, 144(sp)
	# csrw mepc, s0
	
	ld gp,  152(sp)
	ld tp,  160(sp)
	ld s0,  168(sp)
	ld s1,  176(sp)
	ld s2,  184(sp)
	ld s3,  192(sp)
	ld s4,  200(sp)
	ld s5,  208(sp)
	ld s6,  216(sp)
	ld s7,  224(sp)
	ld s8,  232(sp)
	ld s9,  240(sp)
	ld s10,	248(sp)
	ld s11,	256(sp)
    ld sp, 0(sp)

	mret