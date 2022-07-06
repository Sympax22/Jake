#include "pt.h"

/* FIXME: use non-shift version everywhere, i.e., also for gets */
/* EINVALARGS is a bit of a catchall */
#define EINVALARGS 1
#define ENORESOURCES 2
#define ENOTMAPPED 3
#define EMAPPED 4

#define VPN_MASK 0x1FF
#define VPN_MASK_WIDTH 9
#define VPN_SHIFT(level) (VPN_MASK_WIDTH * (level))

#define vpn2vpnlevel(vpn, level) (((vpn) >> VPN_SHIFT(level)) & VPN_MASK)
#define vpn_align(vpn, level) ((vpn) & (-1L << VPN_SHIFT(level)))

/* Some bit magic */
#define __floor2(b, x) ((x) & -(b))
#define __ceil2(b, x) (((x) + (b) + -1) & -(b))

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
#define is_kvirt(virt) (((uintptr_t)virt >> VIRT_MEM_KERNEL_SHIFT) & 0x1)

#define vpn2prevpageboundary(vpn, level) ((vpn) & (-1L * level2pagen(level)))
#define vpn2nextpageboundary(vpn, level)                                       \
	(((vpn) + level2pagen(level) - 1) & (-1L * level2pagen(level)))

extern struct buddy_layout layout;

/* Kernel satp: the kernel's address space */
uintptr_t ksatp;

/*
 * Functions for debugging (see end of this file)
 */
static void pt_pte_print(uintptr_t pte);

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
struct vma *kvmas;

/* 4 KiB, 2 MiB, 1 GiB, 512 GiB */
enum levels { PAGE_NORMAL, PAGE_MEGA, PAGE_GIGA, PAGE_TERA };

/*
 * The page structure can be thought of as the "interface" between VMAs
 * and low-level page table management. It is the package of information that
 * our page table walker needs to create new mappings.
 */
struct page {
	uintptr_t vpn;
	size_t level;
	uintptr_t ppn; /* The PPN is also part of the PTE */
	uintptr_t pte;
};

/*
 * phys2kvirt: converts a physical address to a kernel virtual address: as you
 * can see depending on whether the MMU is enabled, a kernel virtual address
 * may be either a "high" virtual address or actually a "low" physical address
 */
void *phys2kvirt(uintptr_t phys)
{
	return (void *)(VIRT_MEM_KERNEL_BASE * __is_paging_enabled() + phys);
}

/*
 * See pt.h
 */
uintptr_t pt_alloc(size_t level)
{
	struct block *block = buddy_alloc(level2order(level));

	if (!block) return 0;

	memset(phys2kvirt(ppn2phys(block2ppn(block))), 0,
	       PAGE_SIZE * (1UL << level2order(level)));

	return ppn2phys(block2ppn(block));
}

/*
 * See pt.h
 */
int pt_free(uintptr_t phys)
{
	return buddy_free(ppn2block(phys2ppn(phys)));
}

/*
 * See pt.h
 */
void *pt_salloc(size_t size)
{
	static uintptr_t base = 0;
	static uintptr_t cursor = 0;

	if (!size) return NULL;

	/*
	 * For allocations larger than size PAGE_NORMAL, use another allocator
	 */
	if (size > level2size(PAGE_NORMAL)) return NULL;

	if (!base) {
		base = pt_alloc(PAGE_NORMAL);
		if (!base) return NULL;
	}

	if (cursor + size > level2size(PAGE_NORMAL)) {
		base = 0;
		cursor = 0;
		return pt_salloc(size);
	}

	void *ptr = phys2kvirt(base + cursor);
	cursor += size;

	return ptr;
}

/*
 * __pt_next_pte: one step in a page table walk
 *
 * @depth: index into @ptes, starting point of the step
 * @vpn: virtual page number, we need it to determine where to go next
 * @ptes: list of PTEs encountered "along the way"
 *
 * Returns a pointer to the most recent PTE in @ptes, i.e., the result of our
 * step
 *
 */
static uintptr_t *__pt_next_pte(size_t depth, uintptr_t vpn, uintptr_t *ptes)
{
	uintptr_t __pte =
		ppn2phys(pte_get_ppn(ptes[depth])) +
		vpn2vpnlevel(vpn, depth2level(depth + 1)) * sizeof(uintptr_t);

	uintptr_t *pte = phys2kvirt(__pte);

	ptes[depth + 1] = *pte;

	return pte;
}

/*
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
ssize_t pt_get_pte(uintptr_t *satp, uintptr_t vpn, uintptr_t *ptes,
		   uintptr_t **out)
{
	uintptr_t *curr_pte = satp;
	ptes[0] = satp2pte(*satp);

	for (ssize_t depth = 0; depth < PT_NUM_LEVELS; depth++) {
		if (pte_is_leaf(ptes[depth])) {
			if (out) *out = curr_pte;
			return depth;
		} else if (!pte_is_valid(ptes[depth])) {
			return -ENOTMAPPED;
		}

		if (depth < PT_NUM_LEVELS - 1) {
			curr_pte = __pt_next_pte(depth, vpn, ptes);
		}
	}

	return -EINVALARGS;
}

/*
 * pt_alloc_pt: page table allocator
 *
 * Returns a page table entry that points to a newly allocated page table
 */
uintptr_t pt_alloc_pt(void)
{
	uintptr_t pte = 0;
	uintptr_t ppn = phys2ppn(pt_alloc(0));

	if (!ppn) return 0;

	pte = pte_set_ppn(pte, ppn);
	pte = pte_set(pte, PTE_VALID_SHIFT);

	return pte;
}

/*
 * pt_ste_pte_nonleaf: sets a nonleaf page table entry
 *
 * @pte_ptr: a pointer to the page table entry that should be set
 *
 * Returns 0 on success and -error otherwise
 */
static int pt_set_pte_nonleaf(uintptr_t *pte_ptr)
{
	uintptr_t pte = pt_alloc_pt();

	if (!pte) return -ENORESOURCES;

	*pte_ptr = pte;
	pt_flush_tlb();

	return 0;
}

/*
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
static int pt_set_pte_leaf(uintptr_t *satp, uintptr_t vpn, uintptr_t pte,
			   size_t depth)
{
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };

	/* NOTE: pointer to curr. PTE, not to where curr. PTE points to! */
	uintptr_t *curr_pte = satp;
	ptes[0] = satp2pte(*satp);

	/* Let's walk the page table! */
	for (size_t j = 0; j < depth; j++) {
		if (pte_is_leaf(ptes[j])) {
			return -EMAPPED;
		} else if (!pte_is_valid(ptes[j])) {
			/*
			 * In this case, we've found a missing "intermediate" PTE while
			 * we're trying to set a PTE at a lower level
			 */
			if (pt_set_pte_nonleaf(curr_pte)) return -ENORESOURCES;
			ptes[j] = *curr_pte;
		}

		curr_pte = __pt_next_pte(j, vpn, ptes);
	}

	/*
	 * PTE already mapped and we're trying to overwrite it with something
	 * else, but note the `&& pte` to make unmapping possible ;)
	 */
	if (pte_is_valid(ptes[depth]) && pte) return -EMAPPED;

	*curr_pte = pte;
	pt_flush_tlb();
	ptes[depth] = *curr_pte;

	return 0;
}

/*
 * pt_unmap_page: sets the PTE corresponding to the page at @vpn to zero
 *
 * Returns the size, in pagen, of the unmapped page, or -error on failure
 */
static ssize_t pt_unmap_page(uintptr_t *satp, uintptr_t vpn)
{
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };
	uintptr_t *out = NULL;
	ssize_t depth = pt_get_pte(satp, vpn, ptes, &out);

	if (depth < 0) {
		return depth;
	} else {
		*out = 0;
		return level2pagen(depth2level(depth));
	}
}

static ssize_t __pt_find_level_of_highest_empty_pte(uintptr_t *satp,
						    uintptr_t vpn)
{
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };

	/*
	 * This @vpn should not be mapped already. Why might it be mapped
	 * already? If we have been under memory pressure in the past, and not
	 * able to allocate a huge page, we also should not this time. In other
	 * words, this is to make sure that we do not overwrite a deeper page
	 * with a more shallow (larger) one
	 */
	if (pt_get_pte(satp, vpn, ptes, NULL) >= 0) return -EMAPPED;

	/*
	 * Gives us the smallest depth at which there is no PTE, or,
	 * equivalently, the level of the highest empty PTE
	 */
	for (size_t depth = 0; depth < PT_NUM_LEVELS; depth++) {
		if (!ptes[depth]) return depth2level(depth);
	}

	return -EINVALARGS;
}

/*
 * __pt_page_pte_set_perm: sets @page->pte's permissions based on @flags
 */
static void __pt_page_pte_set_perm(struct page *page, uintptr_t flags)
{
	page->pte = flags & VMA_READ ? pte_set(page->pte, PTE_READ_SHIFT) :
				       page->pte;
	page->pte = flags & VMA_WRITE ? pte_set(page->pte, PTE_WRITE_SHIFT) :
					page->pte;
	page->pte = flags & VMA_EXEC ? pte_set(page->pte, PTE_EXEC_SHIFT) :
				       page->pte;
	page->pte = flags & VMA_USER ? pte_set(page->pte, PTE_USER_SHIFT) :
				       page->pte;
	/* FIXME: later */
	/*
	 * in->pte = pte_set(in->pte, PTE_ACCESSED_SHIFT);
	 * in->pte = pte_set(in->pte, PTE_DIRTY_SHIFT);
	 */
}

/*
 * __pt_page_needs_pte: takes care of the PPN/PTE fields of a page structure
 */
static int __pt_page_needs_pte(struct vma *vma, uintptr_t __ppn,
			       struct page *out)
{
	/* 1. Convert VMA flags to PTE permission bits */
	__pt_page_pte_set_perm(out, vma->flags);

	/*
	 * 2. Set the PTE's PPN field: here we decide to either use @__ppn or
	 * (as in the common case) to ask buddy for a PPN
	 */
	if (vma->flags & VMA_POPULATE && vma->flags & VMA_IDENTITY) {
		return -EINVALARGS;
	}
	if (vma->flags & VMA_IDENTITY) {
		out->ppn = __ppn;
	} else {
		out->ppn = phys2ppn(pt_alloc(out->level));

		/*
		 * In theory, 0 is a valid PPN, in practice, that page number
		 * will always be identity mapped, so if we get it here there's
		 * a problem
		 */
		if (!out->ppn) return -ENORESOURCES;
	}

	out->pte = pte_set_ppn(out->pte, out->ppn);
	out->pte = pte_set(out->pte, PTE_VALID_SHIFT);

	return 0;
}

/*
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
	out->vpn = vpn;
	out->level = level;

	/* ... the "hard" part: initializing the PTE */
	return __pt_page_needs_pte(vma, __ppn, out);
}

/*
 * pt_vma_new_page_at_vpn: creates a new page inside @vma at @vpn. Will try to
 * figure out what the page's level should be. Calls
 * pt_vma_new_page_at_vpn_level to figure out its PPN
 *
 * Returns the page's size in terms of PAGE_NORMAL pages (i.e. pagen)
 */
static ssize_t pt_vma_new_page_at_vpn(uintptr_t *satp, struct vma *vma,
				      uintptr_t vpn, uintptr_t __ppn,
				      struct page *out)
{
	ssize_t level = PAGE_TERA;
	size_t end = vma->vpn + vma->pagen;

	/* Requirement 1: make it fit */
	while (!(vma->vpn <= vpn2prevpageboundary(vpn, level)) ||
	       !((vpn2prevpageboundary(vpn, level) + level2pagen(level)) <=
		 end)) {
		level--;
		if (level < 0) return 0;
	}

	/* Requirement 2: no other, existing mappings in the way */
	ssize_t maxlevel = __pt_find_level_of_highest_empty_pte(
		satp, vpn2prevpageboundary(vpn, level));

	if (maxlevel < 0) {
		return maxlevel;
	} else {
		level = level > maxlevel ? maxlevel : level;
	}

	/*
	 * Requirement 3: available physical memory: buddy must be able to give
	 * us a contiguous chunk of physical memory that is large enough
	 */
	while (pt_vma_new_page_at_vpn_of_level(
		vma, vpn2prevpageboundary(vpn, level), __ppn, level, out)) {
		level--;
		if (level < 0) return 0;
	}

	/*
	 * We return the progress made, in pagen, starting from the original
	 * @vpn
	 */
	return vpn2prevpageboundary(vpn, level) + level2pagen(level) - vpn;
}

/*
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
	if (page->level > PAGE_TERA) return -EINVALARGS;

	return pt_set_pte_leaf(satp, page->vpn, page->pte,
			       level2depth(page->level));
}

/*
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
	ssize_t pagen = pt_vma_new_page_at_vpn(satp, vma, vpn, __ppn, &page);

	if (!pagen) return 0;

	int ret = __pt_map_page(satp, &page);

	if (ret) return ret;

	return pagen;
}

/*
 * See pt_vma_new
 */
static ssize_t __pt_vma_new(uintptr_t *satp, uintptr_t vpn, uintptr_t __ppn,
			    size_t pagen, uintptr_t flags, struct vma *out)
{
	out->vpn = vpn;
	out->pagen = pagen;
	out->flags = flags;

	/* We assume that @out is already on a linked list somewhere */
	if (!is_node_init(&out->node)) return -EINVALARGS;

	/*
	 * In these cases, we need to back the VMA by physical memory
	 */
	if (flags & VMA_IDENTITY || flags & VMA_POPULATE) {
		do {
			pagen = pt_vma_map_page_at_vpn(satp, out, vpn, __ppn);

			if (pagen < 0) {
				return pagen;
			} else {
				vpn += pagen;
				/* Only relevant for VMA_IDENTITY */
				__ppn += pagen;
			}
		} while (pagen > 0);
	}

	return 0;
}

/*
 * See pt.h
 *
 * Note how we hide the special case in which VMA_IDENTITY is set (and __ppn
 * used) by creating a wrapper for __pt_vma_new
 */
ssize_t pt_vma_new(uintptr_t *satp, uintptr_t vpn, size_t pagen,
		   uintptr_t flags, struct vma *out)
{
	if (flags & VMA_IDENTITY) {
		return -EINVALARGS;
	} else {
		return __pt_vma_new(satp, vpn, 0, pagen, flags, out);
	}
}

/*
 * See pt.h
 */
void pt_vma_unmap_pages(uintptr_t *satp, struct vma *vma)
{
	uintptr_t base = vma->vpn;
	uintptr_t end = base + vma->pagen;
	ssize_t pagen = 0;

	for (uintptr_t vpn = base; vpn < end; vpn += pagen) {
		pagen = pt_unmap_page(satp, vpn);
		/*
		 * Some parts of the VMA might have already been unmapped, in
		 * that case, just move on to the next page
		 */
		pagen = pagen ? pagen : level2pagen(PAGE_NORMAL);
	}
}

/*
 * See pt.h
 */
void pt_vma_destroy(uintptr_t *satp, struct vma *vma)
{
	/* FIXME: free @vma (requires slab allocator) */
	list_remove(&vma->node);
	pt_vma_unmap_pages(satp, vma);
}

/*
 * ksatp_init: initialized the "kernel satp" (i.e., the root of the kernel's
 * address space/page tables)
 *
 * Does not yet turn on the MMU!
 */
static int ksatp_init(void)
{
	uintptr_t pte = pt_alloc_pt();

	if (!pte) return -1;

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

/*
 * ksatp_enable_paging: turns on the MMU (good luck)
 */
static void ksatp_enable_paging(void)
{
	ksatp = satp_set(ksatp, SATP_MODE_SV48, SATP_MODE_MASK,
			 SATP_MODE_SHIFT);
	csrw(satp, ksatp);
	pt_flush_tlb();
}

/*
 * pt_init_vmas_head: initializes the head of the linked list of kernel/user VMAs,
 * also see kvmas_head declaration above
 */
int pt_init_vmas_head(struct node **vmas_head)
{
	*vmas_head = pt_salloc(sizeof(struct node));

	if (!(*vmas_head)) return -ENORESOURCES;

	node_init(*vmas_head);

	return 0;
}

/*
 * pt_alloc_vma: allocates a new kernel/user VMA structure and adds it to the
 * linked list of kernel/user VMAs, but doesn't initialize it otherwise (that's done
 * by pt_vma_new)
 */
struct vma *pt_alloc_vma(struct node *vmas_head)
{
	struct vma *kvma = pt_salloc(sizeof(struct vma));

	if (!kvma) return NULL;

	node_init(&kvma->node);

	if (!is_node_init(vmas_head)) return NULL;

	list_append(vmas_head, &kvma->node);

	return kvma;
}

/*
 * __map_lowhigh: creates two identity mappings for @ppn to bootstrap paging,
 * one low and one high. The low mapping is removed later by
 * pt_destroy_low_vmas
 */
static int __map_lowhigh(uintptr_t *satp, uintptr_t vpn, uintptr_t ppn,
			 size_t pagen, uintptr_t flags)
{
	struct vma *low = pt_alloc_vma(kvmas_head);

	if (!low) return -ENORESOURCES;

	struct vma *high = pt_alloc_vma(kvmas_head);

	if (!high) return -ENORESOURCES;

	/*
	 * As you can see, we use @ppn here for @vpn, which might be
	 * confusing, however, it just means that vpn = ppn, in other words,
	 * the (low) identity mapping we need to be able to turn on the MMU
	 */
	if (__pt_vma_new(&ksatp, ppn, ppn, pagen, flags, low)) {
		return -EINVALARGS;
	}

	if (__pt_vma_new(&ksatp, vpn, ppn, pagen, flags, high)) {
		return -EINVALARGS;
	}

	return 0;
}

static int map_phys_mem_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t pagen = phys2ppn(layout.phys_end - layout.phys_base);
	uintptr_t ppn = phys2ppn(layout.phys_base);
	uintptr_t vpn = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/*
 * FIXME: special case .data and .bss, we don't want them to be executable or
 * writable
 */
/* Includes meta data for buddy */
static int map_kelf_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? PTE_ACCESSED_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_EXEC | VMA_IDENTITY;
	size_t pagen = phys2ppn(layout.phys_base - layout.kelf_base);
	uintptr_t ppn = phys2ppn(layout.kelf_base);
	uintptr_t vpn = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

static int map_kstack_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t pagen = phys2ppn(layout.kstack_size);
	uintptr_t ppn = phys2ppn(layout.phys_end);
	uintptr_t vpn = virt2vpn(VIRT_MEM_KERNEL_BASE + ppn2phys(ppn));

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

static int map_io_lowhigh(void)
{
	/* FIXME: PTE_GLOBAL_SHIFT? */
	uintptr_t flags = VMA_READ | VMA_WRITE | VMA_IDENTITY;
	size_t pagen = phys2ppn(layout.kelf_base);
	uintptr_t ppn = phys2ppn(0);
	uintptr_t vpn = virt2vpn(VIRT_MEM_KERNEL_BASE);

	return __map_lowhigh(&ksatp, vpn, ppn, pagen, flags);
}

/*
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
		     //  /* Update interrupt handler */
		     //  /* FIXME: update mtvec as well, need M-mode for that */
		     "la t0, trap_handler_s\n\t"
		     "csrw stvec, t0\n\t" ::"I"(VIRT_MEM_KERNEL_SHIFT)
		     : "t0", "t1", "t2");
}

/*
 * See pt.h
 */
struct vma *vpn2vma(struct node *vmas_head, uintptr_t vpn)
{
	struct node *node = vmas_head->next;

	while (node != vmas_head) {
		struct vma *vma = member_to_struct(node, node, struct vma);

		if (vpn >= vma->vpn && vpn < vma->vpn + vma->pagen) return vma;

		node = node->next;
	}

	return NULL;
}

/*
 * pt_migrate_kvmas: migrates kernel VMAs to +@offset, needed before jumping
 * high
 */
void pt_migrate_kvmas(uintptr_t offset)
{
	/* Don't forget the head! */
	kvmas_head = (struct node *)((char *)kvmas_head + offset);
	node_migrate(kvmas_head, offset);

	struct node *node = kvmas_head->next;

	while (node != kvmas_head) {
		struct vma *kvma = member_to_struct(node, node, struct vma);

		node_migrate(&kvma->node, offset);

		node = node->next;
	}
}

void pt_destroy_low_kvmas(void)
{
	struct node *node = kvmas_head->next;
	struct node *next = node->next;

	while (node != kvmas_head) {
		struct vma *kvma = member_to_struct(node, node, struct vma);

		/* We might remove the node from the list! */
		next = node->next;

		if (kvma->vpn < virt2vpn(VIRT_MEM_KERNEL_BASE)) {
			pt_vma_destroy(&ksatp, kvma);
		}

		node = next;
	}
}

inline void pt_flush_tlb(void)
{
	asm volatile("sfence.vma zero, zero\n\t"
		     "fence\n\t"
		     "fence.i\n\t" ::
			     : "memory");
}

int pt_init(void)
{
	if (ksatp_init()) return -1;

	if (pt_init_vmas_head(&kvmas_head)) return -1;

	if (map_phys_mem_lowhigh()) return -1;

	if (map_kstack_lowhigh()) return -1;

	if (map_kelf_lowhigh()) return -1;

	if (map_io_lowhigh()) return -1;

	ksatp_enable_paging();
	/*
	 * If your code survives ksatp_enable_paging, and reaches these lines,
	 * congratulations, that was the hard part!
	 */

	buddy_migrate(VIRT_MEM_KERNEL_BASE);

	pt_migrate_kvmas(VIRT_MEM_KERNEL_BASE);

	return 0;
}

inline void unset_vma_protection()
{
	uint64_t t1;
	csrr(sstatus, t1);
	t1 |= SSTATUS_SUM;
	csrw(sstatus, t1);
}

inline void set_vma_protection()
{
	uint64_t t1;
	csrr(sstatus, t1);
	t1 &= !SSTATUS_SUM;
	csrw(sstatus, t1);
}

/*
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

/*
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
	uintptr_t ptes[PT_NUM_LEVELS] = { 0 };
	ssize_t prev = -1;
	uintptr_t *pte;
	ssize_t depth;

	/* -1 means go to kernel directly */
	uintptr_t vpn = user_vpn_to_skip_to >= 0 ?
				user_vpn_to_skip_to :
				virt2vpn(VIRT_MEM_KERNEL_BASE);

	while (vpn <= virt2vpn(VIRT_MEM_KERNEL_BASE + layout.phys_end +
			       layout.kstack_size)) {
		memset(ptes, 0, sizeof(ptes));

		depth = pt_get_pte(&ksatp, vpn, ptes, &pte);

		/* Page size changes within mapping */
		if (vpn != user_vpn_to_skip_to && depth < 0 && depth != prev) {
			printdbg("---- ---- ", (void *)vpn2virt(vpn));
			printstr("\n\n");
		} else if (depth >= 0 && depth != prev) { /* New mapping */
			printdbg("---- ---- ", (void *)vpn2virt(vpn));
			printdbg("    |     ", (void *)depth2level(depth));
			printstr("    |     \n");
		}

		/* Need to make sure we were not in a mapping */
		if (prev < 0 && vpn > user_vpn_to_skip_to + 256 * PAGE_SIZE &&
		    vpn < virt2vpn(VIRT_MEM_KERNEL_BASE)) {
			/* Jump to just before kernel base */
			vpn = virt2vpn(VIRT_MEM_KERNEL_BASE);
		} else if (depth < 0) {
			vpn += level2pagen(0);
		} else {
			vpn += level2pagen(depth2level(depth));
		}

		prev = depth;
	}
}

/*
 * get_kvirt: returns the kernel virtual address (from the identity map) that maps to the same physical page as the specified user virtual address.
 * 
 * @satp pointer to the user satp
 * @user_virtual user virtual address
 */
inline void *get_kvirt(uintptr_t *satp, void *user_virtual)
{
	uintptr_t ptes[PT_NUM_LEVELS];
	uintptr_t *pte_out;
	ssize_t pt_index;

	pt_index = pt_get_pte(satp, virt2vpn((uintptr_t)user_virtual), ptes,
			      &pte_out);

	if (pt_index < 0) {
		printerr(
			"ERROR: cant find the phys of the app .text segment\n");
		return NULL;
	}

	return phys2kvirt(ppn2phys(pte_get_ppn(ptes[pt_index])));
}
