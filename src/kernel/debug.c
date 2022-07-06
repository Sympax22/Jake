#include "debug.h"
#include "exception.h"

// Note: in print_call_info we are in S-mode, we cannot access
// M-mode registers (would cause an exception).
void print_call_info(void)
{
	uintptr_t tmp;
	// maybe mtime has to be changed addr? b/c it's not MM correctly
	// Actually, probably it has to be virtually mapped ...
	// uintptr_t *mtime = 0x200BFF8, *mtimecmp = 0x2004000;

	csrr(sstatus, tmp);
	printdbg("sstatus: ", (void *)tmp);
	printdbg("SPIE: ", (void *)(tmp & SPIE));
	printdbg("UPIE: ", (void *)(tmp & UPIE));
	printdbg("SPP: ", (void *)((tmp & (1 << 8)) >> 8));
	csrr(sepc, tmp);
	printdbg("sepc: ", (void *)tmp);
	csrr(stval, tmp);
	printdbg("stval: ", (void *)tmp);
	csrr(sip, tmp);
	printdbg("sip: ", (void *)tmp);
	csrr(sie, tmp);
	printdbg("sie: ", (void *)tmp);
	csrr(scause, tmp);
	printdbg("scause: ", (void *)tmp);
	csrr(sscratch, tmp);
	printdbg("sscratch: ", (void *)tmp);
	read_r(sp, tmp);
	printdbg("sp: ", (void *)tmp);
}

void mprint_call_info(void)
{
	uintptr_t tmp;
	uintptr_t *mtime = 0x200BFF8, *mtimecmp = 0x2004000;

	csrr(sscratch, tmp);
	printdbg("sscratch: ", (void *)tmp);
	csrr(mscratch, tmp);
	printdbg("mscratch: ", (void *)tmp);
	csrr(sstatus, tmp);
	printdbg("sstatus: ", (void *)tmp);
	csrr(sepc, tmp);
	printdbg("sepc: ", (void *)tmp);
	csrr(mepc, tmp);
	printdbg("mepc: ", (void *)tmp);
	csrr(stval, tmp);
	printdbg("stval: ", (void *)tmp);
	csrr(mtval, tmp);
	printdbg("mtval: ", (void *)tmp);
	csrr(sip, tmp);
	printdbg("sip: ", (void *)tmp);
	csrr(sie, tmp);
	printdbg("sie: ", (void *)tmp);
	csrr(mie, tmp);
	printdbg("mie: ", (void *)tmp);
	csrr(mstatus, tmp);
	printdbg("mstatus: ", (void *)tmp);
	printdbg("MPIE: ", (void *)(tmp & MPIE));
	printdbg("SPIE: ", (void *)(tmp & SPIE));
	printdbg("UPIE: ", (void *)(tmp & UPIE));
	printdbg("SPP: ", (void *)((tmp & (1 << 8)) >> 8));
	printdbg("MPP: ", (void *)((tmp & (3 << 11)) >> 11));
	csrr(mideleg, tmp);
	printdbg("mideleg: ", (void *)tmp);
	csrr(medeleg, tmp);
	printdbg("medeleg: ", (void *)tmp);
	csrr(mip, tmp);
	printdbg("mip: ", (void *)tmp);
	csrr(scause, tmp);
	printdbg("scause: ", (void *)tmp);
	csrr(mcause, tmp);
	printdbg("mcause: ", (void *)tmp);
	csrr(misa, tmp);
	printdbg("misa: \t\t", (void *)tmp);
	printdbg("mtime: \t\t", (void *)(*mtime));
	printdbg("mtimecmp: \t", (void *)(*mtimecmp));
	csrr(satp, tmp);
	printdbg("satp: ", (void *)tmp);
}

void print_all_registers(void)
{
	uintptr_t tmp;
	read_r(sp, tmp);
	printdbg("sp: ", (void *)tmp);
	read_r(ra, tmp);
	printdbg("ra: ", (void *)tmp);
	read_r(t0, tmp);
	printdbg("t0: ", (void *)tmp);
	read_r(t1, tmp);
	printdbg("t1: ", (void *)tmp);
	read_r(t2, tmp);
	printdbg("t2: ", (void *)tmp);
	read_r(t3, tmp);
	printdbg("t3: ", (void *)tmp);
	read_r(t4, tmp);
	printdbg("t4: ", (void *)tmp);
	read_r(t5, tmp);
	printdbg("t5: ", (void *)tmp);
	read_r(t6, tmp);
	printdbg("t6: ", (void *)tmp);
	read_r(a0, tmp);
	printdbg("a0: ", (void *)tmp);
	read_r(a1, tmp);
	printdbg("a1: ", (void *)tmp);
	read_r(a2, tmp);
	printdbg("a2: ", (void *)tmp);
	read_r(a3, tmp);
	printdbg("a3: ", (void *)tmp);
	read_r(a4, tmp);
	printdbg("a4: ", (void *)tmp);
	read_r(a5, tmp);
	printdbg("a5: ", (void *)tmp);
	read_r(a6, tmp);
	printdbg("a6: ", (void *)tmp);
	read_r(a7, tmp);
	printdbg("a7: ", (void *)tmp);
	read_r(tp, tmp);
	printdbg("tp: ", (void *)tmp);
	read_r(gp, tmp);
	printdbg("gp: ", (void *)tmp);
	read_r(s1, tmp);
	printdbg("s1: ", (void *)tmp);
	csrr(sepc, tmp);
	printdbg("sepc: ", (void *)tmp);
	csrr(satp, tmp);
	printdbg("satp: ", (void *)tmp);
	printstr("tf_user:\n");
	proc_print_frame(tf_user);
}
