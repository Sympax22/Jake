#include "exception.h"

/*
 * DISCLAIMER: SEE pt.c
 */

void print_call_info(void)
{
	uintptr_t tmp;
	csrr(sstatus, tmp);
	printdbg("sstatus: ", (void *)tmp);
	csrr(sepc, tmp);
	printdbg("sepc: ", (void *)tmp);
	csrr(stval, tmp);
	printdbg("stval: ", (void *)tmp);
	/*
	 * csrr(sip, tmp);
	 * printdbg("sip: ", tmp);
	 */
}

int vma_has_perm(struct vma *vma, uintptr_t scause)
{
	switch (scause) {
	case LOAD_ACCESS_FAULT:
	case LOAD_PAGE_FAULT:
		return vma->flags & VMA_READ;
	case STORE_AMO_ACCESS_FAULT:
	case STORE_AMO_PAGE_FAULT:
		return vma->flags & VMA_WRITE;
	case INSTRUCTION_PAGE_FAULT:
		return vma->flags & VMA_EXEC;
	}

	return 0;
}

/*
 * This amazing attribute will make the compiler take care of saving and
 * restoring the register file
 *
 * FIXME: implement A/D bits page fault handler (see manual)
 */
__attribute__((interrupt("supervisor"))) void scall(void)
{
	uintptr_t scause = 0;
	uintptr_t stval = 0;

	/* Page fault handler */
	uintptr_t satp = 0;

	csrr(scause, scause);
	scause &= SCAUSE_EXCEPTION_CODE_MASK;

	if ((scause >> SCAUSE_INTERRUPT_SHIFT) & 0x1) 
	{
		printstr("interrupt! not implemented! hlt!\n");
		goto hlt;
	}

	/*
	 * FIXME: in a different life, use vectored interrupt (i.e., different
	 * functions)
	 */
	switch (scause) 
	{
	case ILLEGAL_INSTRUCTION:
		printstr("illegal instruction\n");
		goto hlt;
	case ENV_CALL_FROM_SMODE:
		printstr("env call from smode\n");
		goto hlt;
	case LOAD_ACCESS_FAULT:
		printstr("load access fault\n"); 
	case LOAD_PAGE_FAULT:
		printstr("load page fault\n"); 
	case STORE_AMO_ACCESS_FAULT:
		printstr("store amo access fault\n"); 
	case STORE_AMO_PAGE_FAULT:
		printstr("store amo page fault\n"); 
	case INSTRUCTION_PAGE_FAULT:
		printstr("instruction page fault\n"); 
		csrr(satp, satp);
		csrr(stval, stval);

		uintptr_t vpn = virt2vpn(stval);
		/*
		 * TODO: use one of the functions in pt.c to find out to which
		 * VMA this VPN belongs (one line)
		 */
		struct vma *vma = vpn2vma(kvmas_head, vpn); 

		/*
		 * 1. No VMA? Hard (or major) page fault
		 * 2. VMA but invalid permissions? Hard (or major) page fault
		 * 3. VMA and valid permissions? Soft (or soft) page fault: handle it
		 */
		if (!vma) 
		{
			goto err_majorpf;
		} 
		else if (!vma_has_perm(vma, scause)) 
		{
			goto err_majorpf;
		} 
		else 
		{
			/*
			 * TODO: VMA and valid permissions, call one of the
			 * functions in pt.c to back the page at @vpn with
			 * physical memory, goto err_noresources if it fails
			 */
			// Warning triggered ; update: warning fixed, problem was the &satp (not satp)
			if (pt_vma_map_page_at_vpn(&satp, vma, vpn, 0) == 0)
			{
				goto err_noresources;
			}
			goto ret;
		}
	default:
		printstr("uncaught exception\n");
		printdbg("scause: ", (void *)scause);
		goto hlt;
	}

	/* NOTE: clear pending bit for exceptions that need that */
err_noresources:
	printstr("no resources\n");
	goto hlt;
err_majorpf:
	printstr("major pf\n");
	goto hlt;
hlt:
	printstr("*** *** ***\n");
	print_call_info();
	printstr("*** hlt ***\n");
	for (;;) {}
ret:
	/* Compiler will restore register file for us, and ret becomes sret */
	return;
}

void mcall(void)
{
}
