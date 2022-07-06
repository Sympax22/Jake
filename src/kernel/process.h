#ifndef PROCESS_H
#define PROCESS_H

#include "pt.h"
#include "elfpars.h"
#include "types.h"
#include "exception.h"

// Description of the status of a process in process_list[]
enum PROCESS_STATUS 
{
	PROCESS_FREE_SLOT, /* The slot is free to be used */
	PROCESS_RUNNING,   /* Usermode is running this process */
	PROCESS_PAUSED,    /* The process will be scheduled to run in usermode */
	PROCESS_RESERVED,
};

// Maximum amount of supported processes in Jake
#define MAX_PROCESSES 256

// Default number of pages for the STACK for usermode applications
#define STACK_PAGES 1

// Starting address for the HEAP of usermode applications
#define USER_HEAP_BASE 0x80006000ULL
#define USER_HEAP_BASE_VPN (USER_HEAP_BASE >> 12ULL)

// Last address for the STACK of usermode applications
#define USER_STACK_BASE ((1ULL << 47ULL) - 1ULL - STACK_PAGES * PAGE_SIZE)
#define USER_STACK_BASE_VPN (USER_STACK_BASE >> 12ULL)

// Converts the SATP register-like value to a virtual address
#define satp2virt(x) (((x & 0xFFFFFFFFUL) << 12) | 0xffff800000000000UL)

#define VERIFY_VMAS()                                                          \
	if ((text_vma->flags & (0x4d)) != 77)                                  \
		printerr("ERROR: Text VMA flags are incorrect\n");             \
	if ((stack_vma->flags & (0x45)) != 69)                                 \
		printerr("ERROR: Stack VMA flags are incorrect\n");

// Loads the context of a trap frame to the CPU registers
#define load_context(trap_frame_src)                                           \
	asm volatile("ld t0, %0\n\t"                                           \
		     "ld  sp, 0(t0)\n\t"                                       \
		     "ld  ra, 8(t0)\n\t"                                       \
		     "ld  t1, 136(t0)\n\t"                                     \
		     "csrw satp, t1\n\t"                                       \
		     "sfence.vma zero, zero\n\t"                               \
		     "fence\n\t"                                               \
		     "fence.i\n\t"                                             \
		     "ld  t1, 144(t0)\n\t"                                     \
		     "addi t1, t1, 4\n\t"                                      \
		     "csrw sepc, t1\n\t"                                       \
		     "ld  t1, 24(t0)\n\t"                                      \
		     "ld  t2, 32(t0)\n\t"                                      \
		     "ld  a0, 40(t0)\n\t"                                      \
		     "ld  a1, 48(t0)\n\t"                                      \
		     "ld  a2, 56(t0)\n\t"                                      \
		     "ld  a3, 64(t0)\n\t"                                      \
		     "ld  a4, 72(t0)\n\t"                                      \
		     "ld  a5, 80(t0)\n\t"                                      \
		     "ld  a6, 88(t0)\n\t"                                      \
		     "ld  a7, 96(t0)\n\t"                                      \
		     "ld  t3, 104(t0)\n\t"                                     \
		     "ld  t4, 112(t0)\n\t"                                     \
		     "ld  t5, 120(t0)\n\t"                                     \
		     "ld  t6, 128(t0)\n\t"                                     \
		     "ld  gp, 152(t0)\n\t"                                     \
		     "ld  tp, 160(t0)\n\t"                                     \
		     "ld  s0, 168(t0)\n\t"                                     \
		     "ld  s1, 176(t0)\n\t"                                     \
		     "ld  s2, 184(t0)\n\t"                                     \
		     "ld  s3, 192(t0)\n\t"                                     \
		     "ld  s4, 200(t0)\n\t"                                     \
		     "ld  s5, 208(t0)\n\t"                                     \
		     "ld  s6, 216(t0)\n\t"                                     \
		     "ld  s7, 224(t0)\n\t"                                     \
		     "ld  s8, 232(t0)\n\t"                                     \
		     "ld  s9, 240(t0)\n\t"                                     \
		     "ld  s10,248(t0)\n\t"                                     \
		     "ld  s11,256(t0)\n\t"                                     \
		     "ld  t0, 16(t0)\n\t"                                      \
		     :                                                         \
		     : "m"(trap_frame_src)                                     \
		     : "memory")
struct trap_frame 
{
	uint64_t sp;   //   0
	uint64_t ra;   //   8
	uint64_t t0;   //  16
	uint64_t t1;   //  24
	uint64_t t2;   //  32
	uint64_t a0;   //  40
	uint64_t a1;   //  48
	uint64_t a2;   //  56
	uint64_t a3;   //  64
	uint64_t a4;   //  72
	uint64_t a5;   //  80
	uint64_t a6;   //  88
	uint64_t a7;   //  96
	uint64_t t3;   // 104
	uint64_t t4;   // 112
	uint64_t t5;   // 120
	uint64_t t6;   // 128
	uint64_t satp; // 136
	uint64_t sepc; // 144 Needed for the context switching
	uint64_t gp;   // 152
	uint64_t tp;   // 160
	uint64_t s0;   // 168
	uint64_t s1;   // 176
	uint64_t s2;   // 184
	uint64_t s3;   // 192
	uint64_t s4;   // 200
	uint64_t s5;   // 208
	uint64_t s6;   // 216
	uint64_t s7;   // 224
	uint64_t s8;   // 232
	uint64_t s9;   // 240
	uint64_t s10;  // 248
	uint64_t s11;  // 256
};

struct process 
{
	unsigned int pid;
	int status; // see enum PROCESS_STATUS
	struct node *uvmas;
	struct trap_frame tf;
	uintptr_t satp;
	struct elf_jake elf;
};

int proc_run_process(char *ptr_elf, uint64_t a3);
void proc_init_process_list();
void proc_turnoff();

char *proc_create_process_vmas(int t, struct elf_jake *elf);

void proc_scheduler(void);
void proc_kill_process(int proc_index);
void context_switch();
void proc_copy_frame(struct trap_frame *dst, struct trap_frame *src);
void proc_print_frame(struct trap_frame *src);

extern struct trap_frame *tf_generic, *tf_user;
extern int proc_running;

extern struct process process_list[];

extern int processes_total;

int proc_copy_binary(char *virt_text, struct elf_jake *elf, int page_index);

void set_sscratch();

#endif