#ifndef SYSCALL_H
#define SYSCALL_H

#include "scalls.h"
#include "types.h"
#include "util.h"
#include "pt.h"
#include "process.h"
#include "elfpars.h"
#include "timer.h"

/* See p. 39 of the ISA */
#define SCAUSE_INTERRUPT_SHIFT 63
#define SCAUSE_EXCEPTION_CODE_MASK ~(1UL << SCAUSE_INTERRUPT_SHIFT)

#define SUPERVISOR_SOFTWARE_INTERRUPT 1
#define MACHINE_SOFTWARE_INTERRUPT 3

#define ILLEGAL_INSTRUCTION 2
#define LOAD_ACCESS_FAULT 5
#define STORE_AMO_ACCESS_FAULT 7
#define ENV_CALL_U 8
#define ENV_CALL_S 9
#define ENV_CALL (ENV_CALL_U | ENV_CALL_S)
#define INSTRUCTION_PAGE_FAULT 12
#define LOAD_PAGE_FAULT 13
#define STORE_AMO_PAGE_FAULT 15

#define MPIE 1 << 7
#define SPIE 1 << 5
#define UPIE 1 << 4

#define MTIE (1UL << 7)

#define ECALL_CLEAR_TIMER_S 2
#define ECALL_ENABLE_MTIE 3
#define ECALL_TURNOFF 4
#define ECALL_TIMER_SETUP 5

// Trap frame offsets w.r.t. the stack
#define OFFSET_TF (264)
#define OFFSET_TF_ULL 264ULL

extern struct vma *kvmas;
extern struct node *kvmas_head;

// Increments the SEPC register by 4
#define fix_sepc                                                               \
	asm volatile("addi sp, sp, -8\n\t"                                     \
		     "sd t1, 0(sp)\n\t"                                        \
		     "csrr t1, sepc\n\t"                                       \
		     "add t1, t1, 4\n\t"                                       \
		     "csrw sepc, t1\n\t"                                       \
		     "ld t1, 0(sp)\n\t"                                        \
		     "addi sp, sp, 8\n\t"                                      \
		     :                                                         \
		     :                                                         \
		     :)

// Increments the MEPC register by 4
#define fix_mepc                                                               \
	asm volatile("addi sp, sp, -8\n\t"                                     \
		     "sd t1, 0(sp)\n\t"                                        \
		     "csrr t1, mepc\n\t"                                       \
		     "add t1, t1, 4\n\t"                                       \
		     "csrw mepc, t1\n\t"                                       \
		     "ld t1, 0(sp)\n\t"                                        \
		     "addi sp, sp, 8\n\t" ::                                   \
			     :)

// Clears the SPP register so that SRET
// does not return to supervisor by mistake
#define clear_spp                                                              \
	uintptr_t SPP = ~(1UL << 8);                                           \
	asm volatile("mv t0, %0\n\t"                                           \
		     "csrr t1, sstatus\n\t"                                    \
		     "and t0, t1, t0\n\t"                                      \
		     "csrw sstatus, t0\n\t"                                    \
		     :                                                         \
		     : "r"(SPP)                                                \
		     : "t0", "t1", "memory")

// Sets the supervisor timer interrupt pending bit
#define trigger_timer_s                                                        \
	asm volatile("csrr t1, mip\n\t"                                        \
		     "li t0, 0x20\n\t"                                         \
		     "or t0, t1, t0\n\t"                                       \
		     "csrw mip, t0\n\t" ::                                     \
			     : "t0", "t1")

// Clears the supervisor timer interrupt pending bit
#define clear_timer_s                                                          \
	asm volatile("csrr t1, mip\n\t"                                        \
		     "li t0, 0x20\n\t"                                         \
		     "not t0, t0\n\t"                                          \
		     "and t0, t1, t0\n\t"                                      \
		     "csrw mip, t0\n\t" ::                                     \
			     : "t0", "t1");

// Memory mapped shut-down
#define turn_off                                                               \
	asm volatile("li t1, 0x100000\n\t"                                     \
		     "li t0, 0x5555\n\t"                                       \
		     "sw t0, 0(t1)\n\t");

void increase_timer();

// Should be equal to syscall from api.h
uint64_t ecall(const char ID, const char arg1, const char arg2);

void ecall_disable_tint();
void ecall_poweroff();
void ecall_timer_setup();

void scheduler(void);
void kill_process(void);

extern int processes_total;

#endif