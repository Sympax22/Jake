#include "process.h"

static int  proc_find_free_process_slot();
static void proc_copy_kernel_pt(uintptr_t satp);
static void copy_elf_struct(struct elf_jake *out, struct elf_jake *in);
static void reset_elf_struct(struct elf_jake *out);

// For set_ssratch
extern struct buddy_layout layout;

extern uintptr_t ksatp;
struct trap_frame *tf_user, *tf_generic;

// Index for the process that is running
int proc_running = 0;

// Total processes currently running
int processes_total = 0;

static unsigned int last_pid = 0;
// Pre-allocated list of processes
struct process process_list[MAX_PROCESSES];

/* TODO 2: Written by us
 * proc_run_process
 * Starts a process by allocating pages for it, setting up the stack and the returning point.
 * @ptr_elf:	pointer to the elf binary location
 * @a3:			argument passed to the application' main function
 */
int proc_run_process(char *ptr_elf, uint64_t a3)
{
	// --- NOT WRITTEN BY US ---
	csrw(satp, ksatp);
	pt_flush_tlb();

	int t = proc_find_free_process_slot();
	if (t < 0)
	{
		return t;
	} 

	// After the next struct to be used is selected,
	// its entries are populated
	process_list[t].pid = last_pid;

	struct elf_jake elf;
	// --- NOT WRITTEN BY US ---


	// TODO: parse the elf binary (1 line)
	// TODO 2.1
	if (parse_elf(ptr_elf, &elf)) 
	{
		return -1;
	}

	// --- NOT WRITTEN BY US ---
	char *virt_stack = proc_create_process_vmas(t, &elf);
	if (virt_stack == NULL)
	{
		return -1;
	} 
	process_list[t].status = PROCESS_RUNNING;

	processes_total++;
	last_pid++;
	proc_running = t;
	copy_elf_struct(&process_list[t].elf, &elf);


	// Set the returning point to the binary' virtual entry
	asm volatile("mv t0, %0\n\t"
		     "csrw sepc, t0\n\t" ::"r"(elf.elf.e_entry)
		     : "t0");

	// Before running the application we set the CPU registers to 0
	// SP becomes the last address of vpn_stack (i.e. the top)
	asm volatile("mv a3, %[a3]\n\t"
		     "mv sp, %[stack]\n\t"
		     "li ra, 0\n\t"
		     "li t0, 0\n\t"
		     "li t1, 0\n\t"
		     "li t2, 0\n\t"
		     "li a0, 0\n\t"
		     "li a1, 0\n\t"
		     "li a2, 0\n\t"
		     "li a4, 0\n\t"
		     "li a5, 0\n\t"
		     "li a6, 0\n\t"
		     "li a7, 0\n\t"
		     "li t3, 0\n\t"
		     "li t4, 0\n\t"
		     "li t5, 0\n\t"
		     "li t6, 0\n\t"
		     "li gp, 0\n\t"
		     "li tp, 0\n\t"
		     "li s0, 0\n\t"
		     "li s1, 0\n\t"
		     "li s2, 0\n\t"
		     "li s3, 0\n\t"
		     "li s4, 0\n\t"
		     "li s5, 0\n\t"
		     "li s6, 0\n\t"
		     "li s7, 0\n\t"
		     "li s8, 0\n\t"
		     "li s9, 0\n\t"
		     "li s10, 0\n\t"
		     "li s11, 0\n\t"
		     "sret\n\t" ::[a3] "r"(a3),
		     [stack] "r"(virt_stack + PAGE_SIZE * STACK_PAGES));

	return 0; // This return will never be executed
	// --- NOT WRITTEN BY US ---
}

/* Not written by us
 * proc_init_process_list
 * Initialize the process_list[] array to default values.
 */
void proc_init_process_list()
{
	for (int t = 0; t < MAX_PROCESSES; t++) 
	{
		process_list[t].pid    = 0;
		process_list[t].status = PROCESS_FREE_SLOT;
	}

	// Kernel process
	// Currently not used
	process_list[0].status = PROCESS_RESERVED;
	process_list[0].satp   = &ksatp;
}

/* Not written by us
 * proc_turnoff
 * Called by syscall when the last running process has been killed. Shutdowns the kernel. It is also called in case of kernel errors.
 * FIXME: maybe for kernel errors we do not want to shutdown?.
 */
void proc_turnoff()
{
	ecall_poweroff();
}

/* Not written by us
 * proc_copy_frame
 * Copies the source trap_frame into the destination one.
 * @dst: destination trap_frame
 * @src: source trap_frame
 */
void proc_copy_frame(struct trap_frame *dst, struct trap_frame *src)
{
	dst->ra   = src->ra;
	dst->sp   = src->sp;
	dst->gp   = src->gp;
	dst->tp   = src->tp;
	dst->t0   = src->t0;
	dst->t1   = src->t1;
	dst->t2   = src->t2;
	dst->s0   = src->s0;
	dst->s1   = src->s1;
	dst->a0   = src->a0;
	dst->a1   = src->a1;
	dst->a2   = src->a2;
	dst->a3   = src->a3;
	dst->a4   = src->a4;
	dst->a5   = src->a5;
	dst->a6   = src->a6;
	dst->a7   = src->a7;
	dst->s2   = src->s2;
	dst->s3   = src->s3;
	dst->s4   = src->s4;
	dst->s5   = src->s5;
	dst->s6   = src->s6;
	dst->s7   = src->s7;
	dst->s8   = src->s8;
	dst->s9   = src->s9;
	dst->s10  = src->s10;
	dst->s11  = src->s11;
	dst->t3   = src->t3;
	dst->t4   = src->t4;
	dst->t5   = src->t5;
	dst->t6   = src->t6;
	dst->satp = src->satp;
	dst->sepc = src->sepc;
}

/* Not written by us
 * proc_print_frame
 * DEBUG utility to print the current frame.
 * @src: trap_frame to print
 */
void proc_print_frame(struct trap_frame *src)
{
	printdbg("sp: \t\t",   src->sp);
	printdbg("ra: \t\t",   src->ra);
	printdbg("t0: \t\t",   src->t0);
	printdbg("t1: \t\t",   src->t1);
	printdbg("t2: \t\t",   src->t2);
	printdbg("a0: \t\t",   src->a0);
	printdbg("a1: \t\t",   src->a1);
	printdbg("a2: \t\t",   src->a2);
	printdbg("a3: \t\t",   src->a3);
	printdbg("a4: \t\t",   src->a4);
	printdbg("a5: \t\t",   src->a5);
	printdbg("a6: \t\t",   src->a6);
	printdbg("a7: \t\t",   src->a7);
	printdbg("t3: \t\t",   src->t3);
	printdbg("t4: \t\t",   src->t4);
	printdbg("t5: \t\t",   src->t5);
	printdbg("t6: \t\t",   src->t6);
	printdbg("gp: \t\t",   src->gp);
	printdbg("tp: \t\t",   src->tp);
	printdbg("s0: \t\t",   src->s0);
	printdbg("s1: \t\t",   src->s1);
	printdbg("s2: \t\t",   src->s2);
	printdbg("s3: \t\t",   src->s3);
	printdbg("s4: \t\t",   src->s4);
	printdbg("s5: \t\t",   src->s5);
	printdbg("s6: \t\t",   src->s6);
	printdbg("s7: \t\t",   src->s7);
	printdbg("s8: \t\t",   src->s8);
	printdbg("s9: \t\t",   src->s9);
	printdbg("s1: \t\t",   src->s10);
	printdbg("s1: \t\t",   src->s11);
	printdbg("satp: \t\t", src->satp);
	printdbg("sepc: \t\t", src->sepc);
}

/* TODO 7: Written by us
 * proc_scheduler
 * 1) Find a paused process to run and 2) switch context
 * The scheduler is called only if more than one process is running
 * Therefore, if no other process is found, an error is triggered.  
 */
inline void proc_scheduler(void)
{
	/*TODO: Implement proc_scheduler.
	* A simple implementation suggestion is to scan linearly process_list[]
	* to find the first paused process. ~20 lines.
	* The index of the new process should be set to the variable proc_running.
	*/
	// TODO 7.1
	//printstr("\nTried scheduling\n");
	int i = proc_running;
	int j = 0;
	
	do
	{
		i  = (i + 1) % MAX_PROCESSES;
		j += 1;
	} while (process_list[i].status != PROCESS_PAUSED && j < MAX_PROCESSES);

	if(j == MAX_PROCESSES)
	{
		printerr("ERROR: Attempted to find another process but only one was running :(");
		return;
	}

	// else, found the next process
	proc_running = i;

	// Not written by us
	process_list[proc_running].status = PROCESS_RUNNING;
	context_switch();
	return;

	/*for(int i = 1; i < MAX_PROCESSES; i++){
		if(process_list[i].status == PROCESS_PAUSED){
			if(i == proc_running) continue;			
			proc_running = i;
			break;
		}
	}
	process_list[proc_running].status = PROCESS_RUNNING;
	context_switch();*/
}

/* Not written by us
 * context_switch 
 * Loads the trap frame of the process indexed by proc_running and
 * start executing it with sret.
 */
inline void context_switch()
{
	void *ptr_frame = &process_list[proc_running].tf;
	load_context(ptr_frame);
	asm volatile("sret\n\t");
}

/* Not written by us
 * proc_kill_process
 * Frees the resources of a process indexed by proc_index.
 * Updates the VMAs and process_list[].
 * @proc_index: index of the process to kill
 */
inline void proc_kill_process(int proc_index)
{
	csrw(satp, ksatp);
	pt_flush_tlb();

	struct node *node = process_list[proc_running].uvmas->next;
	struct node *next = node->next;

	// FIXME: should we free also the head node?
	while (node != process_list[proc_running].uvmas) 
	{
		struct vma *uvma = member_to_struct(node, node, struct vma);

		next = node->next;
		pt_vma_destroy(&process_list[proc_running].satp, uvma);

		node = next;
	}

	process_list[proc_running].pid    = 0;
	process_list[proc_running].status = PROCESS_FREE_SLOT;
	process_list[proc_running].uvmas  = 0; // FIXME: Is this needed?
	process_list[proc_running].satp   = 0;
	reset_elf_struct(&process_list[proc_running].elf);
	process_list[proc_running].elf.ptr_elf = NULL;

	// FIXME: we miss pt_unalloc_pt()
}

/* Not written by us 
 * proc_setup_satp
 * Sets mode bits for a target SATP, contained at root_pt
 */
inline void proc_setup_satp(uintptr_t *satp, uintptr_t root_pt)
{
	*satp = satp_set(*satp, SATP_MODE_BARE, SATP_MODE_MASK, SATP_MODE_SHIFT);
	*satp = satp_set(*satp, pte_get_ppn(root_pt), SATP_PPN_MASK, SATP_PPN_SHIFT);
	*satp = satp_set(*satp, SATP_MODE_SV48, SATP_MODE_MASK, SATP_MODE_SHIFT);
}

/* Not written by us
 * proc_copy_kernel_pt 		
 * Copies the kernel page tables to a target satp.
 */
static inline void proc_copy_kernel_pt(uintptr_t satp)
{
	void *ptr_ksatp = satp2virt(ksatp);
	void *ptr_satp  = satp2virt(satp);

	// Important to put zero in the first slot
	*(uint64_t *)ptr_satp = 0UL;
	memcpy(ptr_satp + 8, ptr_ksatp + 8, 1024 * 4 - 8);
}

/* TODO 3: Written by us
 * proc_create_process_vmas 		
 * Prepare the minimum number of pages for the code segment and STACK_PAGES for the process stack. Each is a different VMA.
 * @t:			index to a free process slot in process_list[]
 * @elf:		pointer to the parsed elf structure 
 * return: pointer to the stack virtual address, NULL in case of error
 * 
 * Diese scheiss kack funktion steigt in Satans Anus.
 * Behobene Fehler: 
 * -flags vom stack muss auch noch EXEC haben
 * -reihenfolge erst text dann stack vma
 * -beschissene hurenmakro war versteckt, braucht STACK_PAGES beim new vma
 */
inline char *proc_create_process_vmas(int t, struct elf_jake *elf)
{	
	// --- NOT WRITTEN BY US ---
	uintptr_t   root_pt; 
	struct vma *text_vma; 
	struct vma *stack_vma;
	uintptr_t   vpn_text, vpn_stack;
	int         text_pages_needed; 
	// --- NOT WRITTEN BY US ---


	/* TODO:
	* Allocate the root page table using pt_alloc_pt. 
	*/
	// TODO 3.1
	root_pt = pt_alloc_pt();
    if(!root_pt)
	{
		printerr("ERROR: null pointer to pt_alloc_pt.\n");
		return NULL;
	}

	// --- NOT WRITTEN BY US ---
	proc_setup_satp(&process_list[t].satp, root_pt);
	proc_copy_kernel_pt(process_list[t].satp);
	// --- NOT WRITTEN BY US ---


	/* TODOs:
	* Prepare the minimum number of pages for the code (vpn_text) and STACK_PAGES page for the stack (vpn_stack).
	* The pages should be Writable, Readable, eXecutable, and for the Usermode. The pages should not be populated.
	* Use pt_alloc_vma() and pt_init_vmas_head()
	* Be careful and remember that the virtual address contains more bits than a VPN! The stack VPN should be USER_STACK_BASE_VPN.
	*/
	// TODO: the first thing is to initialize the user VMAs linked list
	// by using pt_init_vmas_head()
	// TODO 3.2
	int checker1 = pt_init_vmas_head(&process_list[t].uvmas); 
	if(checker1 < 0)
	{
		printerr("ERROR: init_head failed in proc_create_process_vmas.\n");
		return NULL;
	}
	// TODO: the VMAs must be allocated (pt_alloc_vma())
	// TODO 3.3
	text_vma  = pt_alloc_vma(process_list[t].uvmas);
	stack_vma = pt_alloc_vma(process_list[t].uvmas);

	if(text_vma == NULL || stack_vma == NULL)
	{
		printerr("ERROR: pt_alloc_vma failed in proc_create_process_vma.\n");
		return NULL;
	}

	/* TODOs:
	* After the VMAs are allocated, you can create one for the text and one for the stack, by using pt_vma_new(). 
	* Be warry of the flags that you should pass!
	*/
	// TODO 3.4
	uintptr_t flags_stack = 0x0UL;
	uintptr_t flags_text  = 0x0UL;
	flags_stack = VMA_READ | VMA_WRITE | VMA_EXEC | VMA_USER;
	flags_text  = VMA_READ | VMA_WRITE | VMA_EXEC | VMA_USER;

	vpn_stack = USER_STACK_BASE_VPN;
	vpn_text  = (elf->elf.virtual_load) >> PAGE_SHIFT;

	text_pages_needed = elf->elf.size_load / PAGE_SIZE;
	if (((elf->elf.size_load) % PAGE_SIZE) != 0)
	{
		text_pages_needed += 1;
	}

	// stack size <= text size
	    checker1 = pt_vma_new(&process_list[t].satp, vpn_text, text_pages_needed, flags_text, text_vma);
		if(checker1 < 0)
		{
			printerr("ERROR: pt_vma_new text failed.\n");
			return NULL;
		}
	int checker2 = pt_vma_new(&process_list[t].satp, vpn_stack, STACK_PAGES,    flags_stack,  stack_vma);

	if(checker2 < 0)
	{
		printerr("ERROR: pt_vma_new stack failed.\n");
		return NULL;
	}

    // --- NOT WRITTEN BY US ---
	VERIFY_VMAS();

	csrw(satp, process_list[t].satp);
	pt_flush_tlb();
	return (char *)vpn2virt(vpn_stack);
	// --- NOT WRITTEN BY US ---
}

/* TODO 1: Written by us
 * proc_find_free_process_slot 		
 * return: the index of an unused process in process_list[] (between 1 and MAX_PROCESSES).
 * 	       It should return -1 if there are no more free processes available.
 */
static inline int proc_find_free_process_slot()
{
	// TODO: Write in a simple way a loop that finds the first next process to run (max. 10 lines)
	// TODO 1.1
	// processes_total ^= total amount of processes either paused or running
	// if equal to MAX_PROCESSES, then there is no more process that can be added
	
	if(processes_total == MAX_PROCESSES)
	{
		return -1;
	}

	// Between 1 and MAX_PROCESSES
	int count = 1;

	// find the next free process
	while(count != MAX_PROCESSES && process_list[count].status != PROCESS_FREE_SLOT)
	{
		count += 1;
	}

	// if reached the end of the list, no free process
	if(count == MAX_PROCESSES)
	{
		return -1;
	}

	// return index of the next free process
	return count; 
}

/* Not written by us
 * copy_elf_struct
 * copies the content of @in to @out.
 * @in pointer to an existing elf structure (parsed)
 * @out pointer to an existing elf structure to be filled
 */
static void copy_elf_struct(struct elf_jake *out, struct elf_jake *in)
{
	memcpy(out, in, sizeof(struct elf_jake));
}

/* Not written by us
 * reset_elf_struct
 * reset the content of @in to 0.
 * @out pointer to an existing elf structure
 */
static void reset_elf_struct(struct elf_jake *out)
{
	out->elf.e_entry      = 0;
	out->elf.offset_load  = 0;
	out->elf.size_load    = 0;
	out->elf.valid        = 0;
	out->elf.virtual_load = 0;
}

/* TODO 5: Written by us
 * proc_copy_binary
 * Copies the code section from an elf into the virtual text region. *AT MOST*, ONE page at the time.
 * @virt_text destination pointer where the binary is copied (virtual)
 * @elf pointer to the parsed elf header (includes ptr_elf)
 * @page_index index of the elf page to be copied. The page index is w.r.t. ptr_elf. 
 */
inline int proc_copy_binary(char *virt_text, struct elf_jake *elf, int page_index)
{
	// --- NOT WRITTEN BY US ---
	if (virt_text == NULL || elf == NULL || elf->ptr_elf == NULL) 
	{
		printerr("ERROR: null pointer to proc_copy_binary.\n");
		return -1;
	}
	// --- NOT WRITTEN BY US ---

	// TODO: Finish proc_copy_binary.
	// Use memcpy() to copy data. There might be a case where less than a page should be copied.
	// TODO 5.1
	void* dest = (void*) (virt_text);
	uintptr_t size_load   = elf->elf.size_load;
	uintptr_t offset_load = elf->elf.offset_load;

	const void* src = (const void*) (elf->ptr_elf + offset_load + PAGE_SIZE * page_index);
	
	size_t n = PAGE_SIZE;
	int index_size_load   = size_load   / PAGE_SIZE;
	int index_offset_load = offset_load / PAGE_SIZE;

	if(page_index == index_size_load)
	{
		n = size_load % PAGE_SIZE;
	}
	/*else if(page_index > index_size_load)
	{
		return -1;
	}
	else if(page_index < 0)
	{
		return -1;
	}*/

	memcpy(dest, src, n);
	return 0;
}

/* Not written by us
 * set_sscratch
 * Sets the stack pointer for the supervisor mode and the (fixed)
 * pointer to the user trap frame
 */
inline void set_sscratch()
{
	uint64_t tmp_scratch = VIRT_MEM_KERNEL_BASE + layout.phys_end + layout.kstack_size;
	csrw(sscratch, tmp_scratch);
	tf_user = (struct trap_frame *)(tmp_scratch - OFFSET_TF);
}