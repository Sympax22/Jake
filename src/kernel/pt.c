/*
 *
 * /!\ DISCLAIMER /!\
 * 
 * This code was heavily adapted (not modified in content except for the TODOs and some prints)  
 * to allow us to get a better overview of which function is called first. 
 * Please do not be surprised if the order of the functions is not the original order. 
 * 
 * /!\ DISCLAIMER /!\
 * 
 * Happy reading!
 * 
 */

// --- INCLUDES --- //
#include "pt.h"
// --- INCLUDES --- //
// --- DEFINES --- //
/* FIXME: use non-shift version everywhere, i.e., also for gets */
/* EINVALARGS is a bit of a catchall */
#define EINVALARGS       1
#define ENORESOURCES     2
#define ENOTMAPPED       3
#define EMAPPED          4

#define SATP_MODE_BARE 0UL
#define SATP_MODE_SV48 9UL

#define SATP_MODE_SHIFT 60
#define SATP_MODE_MASK 0xF
#define SATP_ASID_SHIFT 44
#define SATP_ASID_MASK 0x1FF

#define VPN_MASK       0x1FF
#define VPN_MASK_WIDTH   9
#define VPN_SHIFT(level) (VPN_MASK_WIDTH * (level))

#define vpn2vpnlevel(vpn, level) (((vpn) >> VPN_SHIFT(level)) & VPN_MASK)
#define    vpn_align(vpn, level) ((vpn) & (-1L << VPN_SHIFT(level)))

/* Some bit magic */
#define __floor2(b, x) ((x) & -(b))
#define  __ceil2(b, x) (((x) + (b) + -1) & -(b))

/*
 * What are "pagen", "level", "depth", and "size"? (And "order", but that one
 * should be familiar.)
 *
 * size: simple, size of something in terms of bytes
 *
 * depth: page table depth (from top to bottom), ranges from 0 up to and
 * including 4, where 0 refers to satp, 1 is the depth of the top-level page
 * table (i.e., the table pointed to by satp), etc. Used to walk page tables
 *
 * level: almost the inverse of depth (see enum levels and level2depth): a page
 * of level PAGE_NORMAL (=0) is a page at depth 4 in the page table, and has
 * size 4 KiB. A page of level PAGE_MEGA (=1) is a page at depth 3 in the page
 * table, and has size 2 MiB, etc.
 *
 * pagen: the size, typically of a range of pages, in terms of number of
 * PAGE_NORMAL pages, in other words: 1 means 4 KiB, 2 means 8 KiB, etc.
 *
 * order: from buddy! As you can see, there's a level2order macro. Do you get
 * why it's so simple and order is just level times 9?
 */
#define level2pagen(level) (1UL << (VPN_MASK_WIDTH * (level)))
#define level2order(level) ((level)*9)
#define pagen2ceillevel(pagen)                                                 \
	(ctz(__ceil2(1L << VPN_MASK_WIDTH, (pagen))) / VPN_MASK_WIDTH)
#define pagen2floorlevel(pagen)                                                \
	(ctz(__floor2(1L << VPN_MASK_WIDTH, (pagen))) / VPN_MASK_WIDTH)
#define level2size(level) (level2pagen(level) * PAGE_SIZE)

#define __is_paging_enabled() (!(((ksatp) >> SATP_MODE_SHIFT) ^ SATP_MODE_SV48))
#define        pte2kvirt(pte) (phys2kvirt(ppn2phys(pte_get_ppn(pte))))
#define        is_kvirt(virt) (((uintptr_t)virt >> VIRT_MEM_KERNEL_SHIFT) & 0x1)

#define vpn2prevpageboundary(vpn, level) ((vpn) & (-1L * level2pagen(level)))
#define vpn2nextpageboundary(vpn, level)                                       \
	(((vpn) + level2pagen(level) - 1) & (-1L * level2pagen(level)))
// --- DEFINES --- //
// --- GLOBALS --- //
extern struct buddy_layout layout;

/* Kernel satp: the kernel's address space */
static uintptr_t ksatp;

/*
 * Functions for debugging (see end of this file)
 */
static void pt_pte_print(uintptr_t pte);
static void pt_print(intptr_t user_vpn_to_skip_to);
void       *pt_salloc(size_t size);

/*
 * Pointer to the head node of a linked list of kernel VMAs. Unlike the
 * non-head nodes, the head node is not part of a VMA structure! This means
 * that calling member_to_struct on the head node will give you trouble (don't
 * worry if you don't understand this the first time you read it, it's not
 * easy).
 *
 * FIXME: should be moved inside a task struct later
 */
struct node *kvmas_head;

/* 4 KiB, 2 MiB, 1 GiB, 512 GiB */
enum levels { PAGE_NORMAL, PAGE_MEGA, PAGE_GIGA, PAGE_TERA };

/*
 * The page structure can be thought of as the "interface" between VMAs
 * and low-level page table management. It is the package of information that
 * our page table walker needs to create new mappings.
 */
struct page 
{
	uintptr_t   vpn;
	size_t    level;
	uintptr_t   ppn; /* The PPN is also part of the PTE */
	uintptr_t   pte;
};
// --- GLOBALS --- //
// ------ PROTOTYPES ------ //
// --- PREWRITTEN --- //
ssize_t          pt_vma_new(uintptr_t *satp, uintptr_t vpn, size_t pagen, uintptr_t flags, struct vma *out);
ssize_t          pt_vma_map_page_at_vpn(uintptr_t *satp, struct vma *vma, uintptr_t vpn, uintptr_t __ppn);
static int       pt_vma_new_page_at_vpn_of_level(struct vma *vma, uintptr_t vpn, uintptr_t __ppn, size_t level, struct page *out);
static int     __pt_map_page(uintptr_t *satp, struct page *page);
static int       pt_set_pte_nonleaf(uintptr_t *pte_ptr);	
static uintptr_t pt_alloc_pt(void);
void             pt_flush_tlb(void);
void            *phys2kvirt(uintptr_t phys);
uintptr_t        pt_alloc(size_t level);		
int              pt_free(uintptr_t phys);
static ssize_t   pt_get_pte(uintptr_t *satp, uintptr_t vpn, uintptr_t *ptes, uintptr_t **out);
static ssize_t   pt_unmap_page(uintptr_t *satp, uintptr_t vpn);	
void             pt_vma_unmap_pages(uintptr_t *satp, struct vma *vma);	
void             pt_vma_destroy(uintptr_t *satp, struct vma *vma);
static int       ksatp_init(void); 
static void      ksatp_enable_paging(void); 
static int       pt_init_kvmas_head(void);
struct vma      *pt_alloc_kvma(void);
static int     __map_lowhigh(uintptr_t *satp, uintptr_t vpn, uintptr_t ppn, size_t pagen, uintptr_t flags);
static int       map_phys_mem_lowhigh(void);
static int       map_kelf_lowhigh(void);
static int       map_kstack_lowhigh(void);
static int       map_io_lowhigh(void);
void           __attribute((noinline)) pt_jump_to_high(void);
struct vma      *vpn2vma(struct node *vmas_head, uintptr_t vpn);
void             pt_migrate_kvmas(uintptr_t offset);
void             pt_destroy_low_kvmas(void);
int              pt_init(void);
// --- PREWRITTEN --- //
// --- TODOs --- //
static ssize_t    __pt_vma_new(uintptr_t *satp, uintptr_t vpn, uintptr_t __ppn, size_t pagen, uintptr_t flags, struct vma *out);
static ssize_t      pt_vma_new_page_at_vpn(struct vma *vma, uintptr_t vpn, uintptr_t __ppn, struct page *out);
static int        __pt_page_needs_pte(struct vma *vma, uintptr_t __ppn, struct page *out);
static void       __pt_page_pte_set_perm(struct page *page, uintptr_t flags);
static int          pt_set_pte_leaf(uintptr_t *satp, uintptr_t vpn, uintptr_t pte, size_t depth);
static uintptr_t *__pt_next_pte(size_t depth, uintptr_t vpn, uintptr_t *ptes);
// --- TODOs --- //
// ------ PROTOTYPES ------ //
// --- BEGINNING --- //
/* NOT WRITTEN BY US
 * See pt.h
 *
 * Note how we hide the special case in which VMA_IDENTITY is set (and __ppn
 * used) by creating a wrapper for __pt_vma_new
 */
ssize_t pt_vma_new(uintptr_t *satp, uintptr_t vpn, size_t pagen,
		   uintptr_t flags, struct vma *out)
{
	if (flags & VMA_IDENTITY) 
		return -EINVALARGS; 
	else 
		return __pt_vma_new(satp, vpn, 0, pagen, flags, out);
}

/* TODO: WRITTEN BY US
 * See pt_vma_new
 */
static ssize_t __pt_vma_new(uintptr_t *satp, uintptr_t vpn, uintptr_t __ppn,
			    size_t pagen, uintptr_t flags, struct vma *out)
{
	out->vpn   = vpn;
	out->pagen = pagen;
	out->flags = flags;

	/* We assume that @out is already on a linked list somewhere */
	if (!is_node_init(&out->node)) 
		return -EINVALARGS;

	/*
	 * In these cases, we need to back the VMA by physical memory
	 */
	if (flags & VMA_IDENTITY || flags & VMA_POPULATE) 
	{
		/*
		 * TODO: use pt_vma_map_page_at_vpn (in a loop) to map the
		 * entire VMA. Don't forget to update @vpn and @__ppn! (The
		 * latter in case VMA_IDENTITY is set)
		 */
		ssize_t mapped = pagen;
		while (mapped > 0)
		{
			mapped = pt_vma_map_page_at_vpn(satp, out, vpn, __ppn);
			
			if(mapped < 0)
			{
				return -1;
			}

			vpn += mapped;

			if(flags & VMA_IDENTITY)
			{
				__ppn += mapped;
			}
		}
	}

	return 0;
}

/* NOT WRITTEN BY US
 * See pt.h
 */
ssize_t pt_vma_map_page_at_vpn(uintptr_t *satp, struct vma *vma, uintptr_t vpn,
			       uintptr_t __ppn)
{
	/* This initializes the struct to all zeroes in C */
	struct page page = { 0 };

	/*
	 * Before we can map the page, we need to find out:
	 *
	 * 1. The size of the page (which "level")
	 * 2. The PPN that we're going to give it
	 *
	 * We do this here (pt_vma_new_page_at_vpn) to keep our mapping
	 * function (__pt_map_page) clean and simple: it can simply map VPN to
	 * PPN without having to "think" too much
	 */
	ssize_t pagen = pt_vma_new_page_at_vpn(vma, vpn, __ppn, &page);

	if (!pagen) 
		return 0;

	int ret = __pt_map_page(satp, &page);

	if (ret) 
	{
		return ret;
	}

	return pagen;
}
// --- BEGINNING --- //
// --- PART 2 --- //
/* TODO: Written by us
 * pt_vma_new_page_at_vpn: creates a new page inside @vma at @vpn. Will try to
 * figure out what the page's level should be. Calls
 * pt_vma_new_page_at_vpn_level to figure out its PPN
 *
 * Returns the page's size in terms of PAGE_NORMAL pages (i.e. pagen)
 */
static ssize_t pt_vma_new_page_at_vpn(struct vma *vma, uintptr_t vpn,
				      uintptr_t __ppn, struct page *out)
{
	/*
	 * TODO: requirement 1: determine the page's max. size (in pagen), our
	 * (virtual) page should fit inside the VMA, but otherwise should be as
	 * large as possible. Return 0 if there's no more space left in the VMA
	 */
	ssize_t new_vma_cursor = vma->vpn + vma->pagen - vpn;

	if(new_vma_cursor <= 0)
	{
		return 0;
	}

	/*
	 * TODO: requirement 2: convert pagen to level: our page can only be
	 * either 4 KiB, 2 MiB, etc.
	 */
	ssize_t level = pagen2floorlevel(new_vma_cursor); /* TODO: = ... */
	
	if(level > 3)
	{
		printdbg("\n\nvma->vpn: ",        (const void *) vma->vpn);
		printdbg("vma->pagen: ",          (const void *) vma->pagen);
		printdbg("vpn: ",                 (const void *) vpn);
		printdbg("new_vma_cursor: ",      (const void *) new_vma_cursor);
		printdbg("level before change: ", (const void *) level);
		// return -1;
	}
	/*
	 * TODO: requirement 3: alignment: the VPN must be aligned to the
	 * page's level
	 *
	 * Actually, the PPN must also be aligned to the page's level. However,
	 * buddy will give us this for free (do you see why?)
	 */
	while(vpn_align(vpn, level) != vpn)
	{
		if(level == 0)
		{
			return level2pagen(0);
		}
		level = level - 1;
	}

	/*
	 * TODO: requirement 4: available physical memory: buddy must be able
	 * to give us a contiguous chunk of physical memory that is large
	 * enough. Call pt_vma_new_page_at_vpn_of_level and decide, based on
	 * its return value, whether you should decrease the page's level
	 */
	while(pt_vma_new_page_at_vpn_of_level(vma, vpn, __ppn, level, out) != 0)
	{
		if(level == 0)
		{
			return level2pagen(0);
		}

		level = level - 1;
	}

	return level2pagen(level);
}

/* NOT WRITTEN BY US
 * pt_vma_new_page_at_vpn_of_level: creates a new page inside @vma at @vpn of
 * size @level and most importantly, sets the PPN in the page's PTE by calling
 * __pt_page_needs_pte
 *
 * Returns the new page via @out
 */
static int pt_vma_new_page_at_vpn_of_level(struct vma *vma, uintptr_t vpn,
					   uintptr_t __ppn, size_t level,
					   struct page *out)
{
	/* The easy part... */
	out->vpn   = vpn;
	out->level = level;

	/* ... the "hard" part: initializing the PTE */
	return __pt_page_needs_pte(vma, __ppn, out);
}

/*	 TODO: Written by us
 * __pt_page_needs_pte: takes care of the PPN/PTE fields of a page structure
 */
static int __pt_page_needs_pte(struct vma *vma, uintptr_t __ppn,
			       struct page *out)
{
	/*
	 * 1. TODO: convert VMA flags to PTE permission bits by calling
	 * __pt_page_pte_set_perm
	 */
	__pt_page_pte_set_perm(out, vma->flags);

	/*
	 * 2. TODO: set the page's PPN field (out->ppn): decide to either use
	 * @__ppn or (as in the common case) to ask buddy for a PPN. Use
	 * pt_alloc to interact with buddy
	 * @@@ PROBLEM IS HERE @@@
	 */
	if(vma->flags & VMA_IDENTITY)
	{	
		// use __ppn
		out->ppn = __ppn;
	}
	else
	{
		// buddy allocate
		out->ppn = phys2ppn(pt_alloc(out->level));
		if(out->ppn == 0)
		{
			printstr("\nAllocation failed!\n");
			return -1;
		}
	}
	
	/*
	 * 3. Update the PTE's PPN field based on out->ppn and make it valid
	 */
	out->pte = pte_set_ppn(out->pte, out->ppn);
	out->pte = pte_set(out->pte, PTE_VALID_SHIFT);

	return 0;
}

/* TODO: Written by us
 * __pt_page_pte_set_perm: sets @page->pte's permissions based on @flags
 */
static void __pt_page_pte_set_perm(struct page *page, uintptr_t flags)
{
	/*
	 * TODO: convert the VMA_READ, VMA_WRITE, and VMA_EXEC flags to
	 * permissions for the PTE. Just update page->pte, nothing else, no
	 * return value
	 *
	 */	
	// page->pte = 0bX...XXXXX
	// flags     = 0bY...YYYYY
	// __UINT64_MAX__                        = 0b1...11111
	// __UINT64_MAX__ << 4                   = 0b1...10000
	// __UINT64_MAX__ << 4 + 1               = 0b1...10001
	// page->pte & (__UINT64_MAX__ << 4 + 1) = 0bX...X000X
	page->pte = page->pte & ((__UINT64_MAX__ << 4) + 1);

	// __UINT64_MAX__ << 3                                       = 0b1...11000
	// (__UINT64_MAX__ << 3) ^ __UINT64_MAX__                    = 0b0...00111
	// flags & ((__UINT64_MAX__ << 3) ^ (__UINT64_MAX__))        = 0b0...00YYY
	// (flags & ((__UINT64_MAX__ << 3) ^ (__UINT64_MAX__))) << 1 = 0b0...0YYY0
	uintptr_t extracted_flags = (flags & ((__UINT64_MAX__ << 3) ^ (__UINT64_MAX__))) << 1;
	
	// page->pte | flags = 0bX...XYYYX
	page->pte = page->pte | extracted_flags;
}
// --- PART 2 --- //
// --- PART 1 --- //
/* NOT WRITTEN BY US
 * pt_page_map: maps a single page, wrapper for pt_set_pte_leaf
 *
 * @satp: page table
 * @in: page struct to map
 *
 * Return 0 on success and -error on failure
 *
 * You may wonder, why wrap pt_set_pte_leaf? Because we're making a conceptual
 * jump: mapping something actually means setting a PTE in some address space
 */
static int __pt_map_page(uintptr_t *satp, struct page *page)
{
	if (page->level > PAGE_TERA) 
	{
		printdbg("Faulty level: ", (const void *) page->level);
		return -EINVALARGS;
	}

	return pt_set_pte_leaf(satp, page->vpn, page->pte,
			       level2depth(page->level));
}

/* TODO: WRITTEN BY US
 * pt_set_pte_leaf: page table walker
 *
 * @satp: pointer to the page table root
 * @vpn: virtual page number, need it to find out where to go in our page table
 * @pte: the new page table entry, will replace @ptes[depth] in the page table
 * @depth: depth of entry into @ptes that the caller wishes to update
 *
 * Returns 0 on success and -error on failure
 *
 */
static int pt_set_pte_leaf(uintptr_t *satp, uintptr_t vpn, uintptr_t pte, size_t depth)
{
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };

	/* NOTE: pointer to current PTE, not to where current PTE points to! */
	uintptr_t *curr_pte = satp;
	ptes[0] = satp2pte(*satp);

	/* Let's walk the page table! */
	for (size_t j = 0; j < depth; j++) 
	{
		/*
		 * TODO: implement the walk, think of the following cases:
		 *
		 * 1. ptes[j] is a leaf. This is a problem, why?
		 */
		// Is leaf if bit 1 (not bit 0), which is R, is set
		// OR with 0 to extract, if 1 then return -error
		if (pte_is_leaf(ptes[j]))
		{
			// R = 1 meaning read permission meaning leaf, but not max depth!
			// what should be returned here??
			printstr("pte is leaf\n");
			return -1;
		}

		/*
		 * 2. ptes[j] is invalid. This is not a problem, but requires
		 * us to do some extra work before we can move on. Hint: use
		 * pt_set_pte_nonleaf()
		 */
		if (!pte_is_valid(ptes[j]))
		{
			// V = 0 so not valid meaning empty
			if(pt_set_pte_nonleaf(curr_pte) != 0)
			{
				printstr("pte is nonleaf unvalid\n\n");
				return -1;
			}
			// Set V = 1
			ptes[j] = *curr_pte;
		}

		/* 
		 * 3. ptes[j] looks good, that is, it's a valid nonleaf PTE.
		 * Time to take a step! Use __pt_next_pte here
		 */
		curr_pte = __pt_next_pte(j, vpn, ptes);
	}

	*curr_pte = pte;
	pt_flush_tlb();
	ptes[depth] = *curr_pte;

	return 0;
}

/* NOT WRITTEN BY US
 * pt_ste_pte_nonleaf: sets a nonleaf page table entry
 *
 * @pte_ptr: a pointer to the page table entry that should be set
 *
 * Returns 0 on success and -error otherwise
 */
static int pt_set_pte_nonleaf(uintptr_t *pte_ptr)
{
	uintptr_t pte = pt_alloc_pt();

	if (!pte) 
		return -ENORESOURCES;

	*pte_ptr = pte;
	pt_flush_tlb();

	return 0;
}

/* TODO: WRITTEN BY US
 * __pt_next_pte: one step in a page table walk
 *
 * @depth: index into @ptes, starting point of the step
 * @vpn: virtual page number, we need it to determine where to go next
 * @ptes: list of PTEs encountered "along the way"
 *
 * Returns a pointer to the most recent PTE in @ptes, i.e., the result of our step
 */
static uintptr_t *__pt_next_pte(size_t depth, uintptr_t vpn, uintptr_t *ptes)
{
	/*
	 * TODO: make a step in the page table walk. This is difficult. Hint:
	 * first construct a physical pointer to the next PTE, then convert
	 * that pointer to a kernel virtual address, and finally, access it
	 * (dereference it) and store it at depth + 1 in @ptes. Return the
	 * pointer that you dereferenced in the final step
	 *
	 * You will need the following macros: ppn2phys, pte_get_ppn,
	 * vpn2vpnlevel, depth2level, and phys2kvirt
	 *
	 * The good news: just four lines of code: one addition, a call to
	 * phys2kvirt, one dereference (don't forget to update @ptes), and a
	 * return
	 */	

	// Physical pointer to next PTE: 
	// Given: physical pointer to next level

	// Given: virtual offset to pte in next level
	// We construct the physical pointer as follows:
	// ptes[depth] holds the pte that gets converted to ppn
	// ppn gets converted to phys addr
	// We now have a pointer to the first entry of the next table
	// To this, we need to add the line of the wanted next pte, which is exactly the vpn at that depth
	// We thus convert the depth to the corresponding level to convert level and vpn to the correct addand
	uintptr_t phys_next_pte  =  ppn2phys(pte_get_ppn(ptes[depth])) + 8 * vpn2vpnlevel(vpn, depth2level(depth + 1));

	// Next, we convert the physical address to a virutal one and save it as a pointer
	uintptr_t* vp_next_pte   =  phys2kvirt(phys_next_pte);
	
	// The pte @ depth + 1 saved in the ptes becomes the entry the pointer points too
	ptes[depth + 1]          = *vp_next_pte;

	// We return the pointer
	return vp_next_pte;
}
// --- PART 2 --- //

/* NOT WRITTEN BY US
 * pt_alloc_pt: page table allocator
 *
 * Returns a page table entry that points to a newly allocated page table
 */
static uintptr_t pt_alloc_pt(void)
{
	uintptr_t pte = 0;
	uintptr_t ppn = phys2ppn(pt_alloc(0));

	if (!ppn) 
		return 0;

	pte = pte_set_ppn(pte, ppn);
	pte = pte_set(pte, PTE_VALID_SHIFT);

	return pte;
}

/* NOT WRITTEN BY US
 *
 */
void pt_flush_tlb(void)
{
	asm volatile("sfence.vma zero, zero");
}

/* NOT WRITTEN BY US
 * phys2kvirt: converts a physical address to a kernel virtual address: as you
 * can see depending on whether the MMU is enabled, a kernel virtual address
 * may be either a "high" virtual address or actually a "low" physical address
 */
void *phys2kvirt(uintptr_t phys)
{
	return (void *)(VIRT_MEM_KERNEL_BASE * __is_paging_enabled() + phys);
}

/* NOT WRITTEN BY US
 * See pt.h
 */
uintptr_t pt_alloc(size_t level)
{
	struct block *block = buddy_alloc(level2order(level));

	if (!block) 
		return 0;

	memset(phys2kvirt(ppn2phys(block2ppn(block))), 0, PAGE_SIZE * (1UL << level2order(level)));

	return ppn2phys(block2ppn(block));
}

/* NOT WRITTEN BY US
 * See pt.h
 */
int pt_free(uintptr_t phys)
{
	return buddy_free(ppn2block(phys2ppn(phys)));
}

/* NOT WRITTEN BY US
 * See pt.h
 */
void *pt_salloc(size_t size)
{
	static uintptr_t base   = 0;
	static uintptr_t cursor = 0;

	if (!size) 
		return NULL;

	/*
	 * For allocations larger than size PAGE_NORMAL, use another allocator
	 */
	if (size > level2size(PAGE_NORMAL)) 
		return NULL;

	if (!base) 
	{
		base = pt_alloc(PAGE_NORMAL);
		if (!base) 
			return NULL;
	}

	if (cursor + size > level2size(PAGE_NORMAL)) 
	{
		base   = 0;
		cursor = 0;
		return pt_salloc(size);
	}

	void *ptr = phys2kvirt(base + cursor);
	cursor   += size;

	return ptr;
}

/* NOT WRITTEN BY US
 * pt_get_pte: gets the PTE corresponding to @vpn
 *
 * @satp: address space to search in
 * @vpn: virtual page number to look for
 * @ptes: array of PTEs that will be filled along the way, i.e. page walk
 *
 * @out: will make the pointer you give it point to the PTE it found (which can
 * be used to then change it)
 *
 * Returns the depth at which the PTE was found, can be used to index
 * into @ptes
 *
 */
static ssize_t pt_get_pte(uintptr_t *satp, uintptr_t vpn, uintptr_t *ptes,
			  uintptr_t **out)
{
	uintptr_t *curr_pte = satp;
	ptes[0] = satp2pte(*satp);

	for (ssize_t depth = 0; depth < PT_NUM_LEVELS; depth++) 
	{
		if (pte_is_leaf(ptes[depth])) 
		{
			if (out) 
				*out = curr_pte;

			return depth;
		} 
		else if (!pte_is_valid(ptes[depth])) 
		{
			return -ENOTMAPPED;
		}

		if (depth < PT_NUM_LEVELS - 1) 
		{
			curr_pte = __pt_next_pte(depth, vpn, ptes);
		}
	}

	return -EINVALARGS;
}

/* NOT WRITTEN BY US
 * pt_unmap_page: sets the PTE corresponding to the page at @vpn to zero
 *
 * Returns the size, in pagen, of the unmapped page, or -error on failure
 */
static ssize_t pt_unmap_page(uintptr_t *satp, uintptr_t vpn)
{
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };
	uintptr_t *out = NULL;
	ssize_t depth  = pt_get_pte(satp, vpn, ptes, &out);

	if (depth < 0) 
	{
		return depth;
	} 
	else 
	{
		*out = 0;
		return level2pagen(depth2level(depth));
	}
}

/* NOT WRITTEN BY US
 * See pt.h
 */
void pt_vma_unmap_pages(uintptr_t *satp, struct vma *vma)
{
	uintptr_t base = vma->vpn;
	uintptr_t end  = base + vma->pagen;
	ssize_t pagen  = 0;

	for (uintptr_t vpn = base; vpn < end; vpn += pagen) 
	{
		pagen = pt_unmap_page(satp, vpn);
		/*
		 * Some parts of the VMA might have already been unmapped, in
		 * that case, just move on to the next page
		 */
		pagen = pagen ? pagen : level2pagen(PAGE_NORMAL);
	}
}

/* NOT WRITTEN BY US
 * See pt.h
 */
void pt_vma_destroy(uintptr_t *satp, struct vma *vma)
{
	/* FIXME: free @vma (requires slab allocator) */
	list_remove(&vma->node);
	pt_vma_unmap_pages(satp, vma);
}

/* NOT WRITTEN BY US
 * ksatp_init: initialized the "kernel satp" (i.e., the root of the kernel's
 * address space/page tables)
 *
 * Does not yet turn on the MMU!
 */
static int ksatp_init(void)
{
	uintptr_t pte = pt_alloc_pt();

	if (!pte) 
		return -1;

	/*
	 * Set the MODE field to "Bare": "No translation or protection" (not
	 * yet)
	 */
	ksatp = satp_set(ksatp, SATP_MODE_BARE, SATP_MODE_MASK,
			 SATP_MODE_SHIFT);

	/*
	 * Set the PPN field to a newly allocated ppn, the ppn that we will be
	 * using to construct our kernel page tables
	 */
	ksatp = satp_set(ksatp, pte_get_ppn(pte), SATP_PPN_MASK,
			 SATP_PPN_SHIFT);

	return 0;
}


/* NOT WRITTEN BY US
 * pt_print: "page table" print, extremely useful for debugging!
 *
 * Will print the mappings under the kernel's address space, prints for each
 * region the level of the pages in that region, adds a "---- ----" if the
 * level of the pages changes
 *
 * FIXME: this one is ugly
 */
static void pt_print(intptr_t user_vpn_to_skip_to)
{
	uintptr_t  ptes[PT_NUM_LEVELS] = { 0 };
	ssize_t    prev = -1;
	uintptr_t *pte;
	ssize_t    depth;

	/* -1 means go to kernel directly */
	uintptr_t vpn = (user_vpn_to_skip_to >= 0 ? user_vpn_to_skip_to : virt2vpn(VIRT_MEM_KERNEL_BASE));

	while (vpn <= virt2vpn(VIRT_MEM_KERNEL_BASE + layout.phys_end + layout.kstack_size)) 
	{
		memset(ptes, 0, sizeof(ptes));

		depth = pt_get_pte(&ksatp, vpn, ptes, &pte);

		/* Page size changes within mapping */
		if (vpn != user_vpn_to_skip_to && depth < 0 && depth != prev) 
		{
			printdbg("---- ---- ", (void *)vpn2virt(vpn));
			printstr("\n\n");
		} 
		else if (depth >= 0 && depth != prev) 
		{ 
			/* New mapping */
			printdbg("---- ---- ", (void *)vpn2virt(vpn));
			printdbg("    |     ", (void *)depth2level(depth));
			printstr("    |     \n");
		}

		/* Need to make sure we were not in a mapping */
		if (prev < 0 && vpn > user_vpn_to_skip_to + 256 * PAGE_SIZE && vpn < virt2vpn(VIRT_MEM_KERNEL_BASE)) 
		{
			/* Jump to just before kernel base */
			vpn = virt2vpn(VIRT_MEM_KERNEL_BASE);
		} 
		else if (depth < 0) 
		{
			vpn += level2pagen(0);
		} 
		else 
		{
			vpn += level2pagen(depth2level(depth));
		}

		prev = depth;
	}
}

/* NOT WRITTEN BY US
 * ksatp_enable_paging: turns on the MMU (good luck)
 */
static void ksatp_enable_paging(void)
{
	ksatp = satp_set(ksatp, SATP_MODE_SV48, SATP_MODE_MASK,
			 SATP_MODE_SHIFT);

	csrw(satp, ksatp);	
	pt_flush_tlb();
}

/* NOT WRITTEN BY US
 * pt_init_kvmas_head: initializes the head of the linked list of kernel VMAs,
 * also see kvmas_head declaration above
 */
static int pt_init_kvmas_head(void)
{
	kvmas_head = pt_salloc(sizeof(struct node));

	if (!kvmas_head) 
		return -ENORESOURCES;

	node_init(kvmas_head);

	return 0;
}

/* NOT WRITTEN BY US
 * pt_alloc_kvma: allocates a new kernel VMA structure and adds it to the
 * linked list of kernel VMAs, but doesn't initialize it otherwise (that's done
 * by pt_vma_new)
 */
struct vma *pt_alloc_kvma(void)
{
	struct vma *kvma = pt_salloc(sizeof(struct vma));

	if (!kvma) 
		return NULL;

	node_init(&kvma->node);

	if (!is_node_init(kvmas_head)) 
		return NULL;

	list_append(kvmas_head, &kvma->node);

	return kvma;
}

/* NOT WRITTEN BY US
 * __map_lowhigh: creates two identity mappings for @ppn to bootstrap paging,
 * one low and one high. The low mapping is removed later by
 * pt_destroy_low_vmas
 */
static int __map_lowhigh(uintptr_t *satp, uintptr_t vpn, uintptr_t ppn,
			 size_t pagen, uintptr_t flags)
{
	struct vma *low = pt_alloc_kvma();

	if (!low) 
		return -ENORESOURCES;

	struct vma *high = pt_alloc_kvma();

	if (!high) 
		return -ENORESOURCES;

	/*
	 * As you can see, we use @ppn here for @vpn, which might be
	 * confusing, however, it just means that vpn = ppn, in other words,
	 * the (low) identity mapping we need to be able to turn on the MMU
	 */
	if (__pt_vma_new(&ksatp, ppn, ppn, pagen, flags, low)) 
	{
		return -EINVALARGS;
	}

	if (__pt_vma_new(&ksatp, vpn, ppn, pagen, flags, high)) 
	{
		return -EINVALARGS;
	}

	return 0;
}

/* NOT WRITTEN BY US
 *
 */
static int map_phys_mem_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t    pagen = phys2ppn(layout.phys_end - layout.phys_base);
	uintptr_t ppn   = phys2ppn(layout.phys_base);
	uintptr_t vpn   = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/* NOT WRITTEN BY US
 * FIXME: special case .data and .bss, we don't want them to be executable or
 * writable
 */
/* Includes meta data for buddy */
static int map_kelf_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? PTE_ACCESSED_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_EXEC | VMA_IDENTITY;
	size_t    pagen = phys2ppn(layout.phys_base - layout.kelf_base);
	uintptr_t ppn   = phys2ppn(layout.kelf_base);
	uintptr_t vpn   = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/* NOT WRITTEN BY US
 *
 */
static int map_kstack_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t    pagen = phys2ppn(layout.kstack_size);
	uintptr_t ppn   = phys2ppn(layout.phys_end);
	uintptr_t vpn   = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/* NOT WRITTEN BY US
 *
 */
static int map_io_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t    pagen = phys2ppn(layout.kelf_base);
	uintptr_t ppn   = phys2ppn(0);
	uintptr_t vpn   = virt2vpn(VIRT_MEM_KERNEL_BASE);

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/* NOT WRITTEN BY US
 * pt_jump_to_high: jumps from low to high memory, after which the low mappings
 * can be removed
 *
 * Did you know that Jump (For My Love) was written by the Pointer Sisters?
 */
void __attribute((noinline)) pt_jump_to_high(void)
{
	/* FIXME: get rid of 0x1ffff */
	asm volatile("mv t2, ra\n\t" /* Save return address (jalr changes it) */
		     "li t0, 0x1ffff\n\t" /* Add VIRT_MEM_KERNEL_BASE ... */
		     "slli t0, t0, %0\n\t"
		     "auipc t1, 0x0\n\t" /* ... to pc */
		     "add t1, t1, 0x10\n\t" /* Add extra to get past jalr */
		     "add t1, t0, t1\n\t"
		     "jalr t1\n\t"
		     "add ra, t2, t0\n\t" /* Update return address */
		     "add sp, sp, t0\n\t" /* Update stack pointer */
		     "add s0, s0, t0\n\t" /* Update frame pointer */
		     /* Update interrupt handler */
		     /* FIXME: update mtvec as well, need M-mode for that */
		     "la t0, scall\n\t"
		     "csrw stvec, t0\n\t" ::"I"(VIRT_MEM_KERNEL_SHIFT)
		     : "t0", "t1", "t2");
}

/* NOT WRITTEN BY US
 * See pt.h
 */
struct vma *vpn2vma(struct node *vmas_head, uintptr_t vpn)
{
	struct node *node = vmas_head->next;

	while (node != vmas_head) 
	{
		struct vma *vma = member_to_struct(node, node, struct vma);

		if (vpn >= vma->vpn && vpn < vma->vpn + vma->pagen) 
			return vma;

		node = node->next;
	}

	return NULL;
}

/* NOT WRITTEN BY US
 * pt_migrate_kvmas: migrates kernel VMAs to +@offset, needed before jumping
 * high
 */
void pt_migrate_kvmas(uintptr_t offset)
{
	/* Don't forget the head! */
	kvmas_head = (struct node *)((char *)kvmas_head + offset);
	node_migrate(kvmas_head, offset);

	struct node *node = kvmas_head->next;

	while (node != kvmas_head) 
	{
		struct vma *kvma = member_to_struct(node, node, struct vma);

		node_migrate(&kvma->node, offset);

		node = node->next;
	}

	return;
}

/* NOT WRITTEN BY US
 *
 */
void pt_destroy_low_kvmas(void)
{
	struct node *node = kvmas_head->next;
	struct node *next = node->next;

	while (node != kvmas_head) 
	{
		struct vma *kvma = member_to_struct(node, node, struct vma);

		/* We might remove the node from the list! */
		next = node->next;

		if (kvma->vpn < virt2vpn(VIRT_MEM_KERNEL_BASE)) 
		{
			pt_vma_destroy(&ksatp, kvma);
		}

		node = next;
	}
}

/* NOT WRITTEN BY US
 *
 */
int pt_init(void)
{
	if (ksatp_init())           return -1;

	if (pt_init_kvmas_head())   return -1;

	if (map_phys_mem_lowhigh()) return -1; 

	if (map_kstack_lowhigh())   return -1;

	if (map_kelf_lowhigh())     return -1;

	if (map_io_lowhigh())       return -1;

	printstr("\nAbout to turn on the MMU (yikes!), from this point on any memory address will be interpreted as being virtual and trigger a page table walk, I will probably hang and crash silently if you didn't yet implement all the TODOs...\n");

	ksatp_enable_paging();

	/*
	 * If your code survives ksatp_enable_paging, and reaches these lines,
	 * congratulations, that was the hard part!
	 */

	buddy_migrate(VIRT_MEM_KERNEL_BASE);

	pt_migrate_kvmas(VIRT_MEM_KERNEL_BASE);

	return 0;
}

/* NOT WRITTEN BY US
 * pt_pte_print: prints the fields of a PTE, use it for debugging!
 */
static void pt_pte_print(uintptr_t pte)
{
	printdbg("reser: ", (void *)((pte >> 53) & 0x3ff));
	printdbg("ppn  : ", (void *)pte_get_ppn(pte));
	printdbg("(phy): ", (void *)ppn2phys(pte_get_ppn(pte)));
	printdbg("rsw  : ",
		 (void *)((pte >> __PTE_RSW_SHIFT) & __PTE_RSW_MASK));
	printdbg("dirty: ", (void *)pte_get(pte, PTE_DIRTY_SHIFT));
	printdbg("acc. : ", (void *)pte_get(pte, PTE_ACCESSED_SHIFT));
	printdbg("glob.: ", (void *)pte_get(pte, PTE_GLOBAL_SHIFT));
	printdbg("user : ", (void *)pte_get(pte, PTE_USER_SHIFT));
	printdbg("exec : ", (void *)pte_get(pte, PTE_EXEC_SHIFT));
	printdbg("write: ", (void *)pte_get(pte, PTE_WRITE_SHIFT));
	printdbg("read : ", (void *)pte_get(pte, PTE_READ_SHIFT));
	printdbg("valid: ", (void *)pte_get(pte, PTE_VALID_SHIFT));
}