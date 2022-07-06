#ifndef PT_H
#define PT_H

#include "types.h"
#include "util.h"
#include "buddy.h"

// Required to allow the SUPERVISOR
// to read/write user pages
#define SSTATUS_SUM 0x40000ULL // (1UL<<18)

#define SATP_MODE_BARE 0UL
#define SATP_MODE_SV48 9UL
#define SATP_MODE_SHIFT 60
#define SATP_MODE_MASK 0xF

#define PT_NUM_LEVELS 5 /* Including SATP */

#define PAGE_SHIFT 12
#define PAGE_SIZE (0x1UL << PAGE_SHIFT)
#define PAGE_OFFSET_MASK 0xFFF

/* Base of kernel virtual address subspace */
#define VIRT_MEM_KERNEL_SHIFT 47
#define VIRT_MEM_KERNEL_BASE (0x1ffffUL << VIRT_MEM_KERNEL_SHIFT)

#define VMA_READ 0x1UL
#define VMA_WRITE (0x1UL << 2)
#define VMA_EXEC (0x1UL << 3)
#define VMA_POPULATE (0x1UL << 4)
#define VMA_IDENTITY (0x1UL << 5)
#define VMA_USER (0x1UL << 6)

#define PTE_VALID_SHIFT 0
#define PTE_READ_SHIFT 1
#define PTE_WRITE_SHIFT 2
#define PTE_EXEC_SHIFT 3
#define PTE_USER_SHIFT 4
#define PTE_GLOBAL_SHIFT 5 /* Mapping exists in all address spaces (optimis.) */
#define PTE_ACCESSED_SHIFT 6
#define PTE_DIRTY_SHIFT 7

#define __PTE_RSW_SHIFT 8
#define __PTE_RSW_MASK 0x3
#define __PTE_PERM_MASK 0x3FF
#define __PTE_PPN_SHIFT 10
#define __PTE_PPN_MASK 0xFFFFFFFFFFF

#define SATP_PPN_SHIFT 0
#define SATP_PPN_MASK 0xFFFFFFFFFFF

#define level2depth(level) (PT_NUM_LEVELS - 1 - (level))
#define depth2level(depth) level2depth(depth)

#define pte_get(pte, shift) (((pte) >> (shift)) & 0x1)
#define pte_set(pte, shift) ((pte) | (0x1UL << (shift)))
#define pte_unset(pte, shift) (~pte_set(~(pte), (shift)))

#define flags_get(flags, shift) pte_get(flags, shift)
#define flags_set(flags, shift) pte_set(flags, shift)

#define pte_get_ppn(pte) (((pte) >> __PTE_PPN_SHIFT) & __PTE_PPN_MASK)

/* clang-format off */
#define pte_clear_ppn(pte) ((pte) ^ (pte_get_ppn(pte) << __PTE_PPN_SHIFT))
#define pte_set_ppn(pte, new)                                                  \
	(pte_clear_ppn(pte) ^ (((new) & __PTE_PPN_MASK) << __PTE_PPN_SHIFT))
#define satp_clear(satp, mask, shift)                                          \
	((satp) ^ (satp_get(satp, mask, shift) << shift))
#define satp_set(satp, new, mask, shift)                                       \
	(satp_clear(satp, mask, shift) ^ (((new) & (mask)) << (shift)))
/* clang-format on */

#define satp_get(satp, mask, shift) (((satp) >> (shift)) & (mask))
#define satp2pte(satp)                                                         \
	pte_set_ppn(0x1UL, satp_get(satp, SATP_PPN_MASK, SATP_PPN_SHIFT))

#define pte_is_leaf(pte) (pte_is_leaf_read(pte)|pte_is_leaf_exec(pte))
#define pte_is_leaf_read(pte) pte_get(pte, PTE_READ_SHIFT)
#define pte_is_leaf_exec(pte) pte_get(pte, PTE_EXEC_SHIFT)
#define pte2kvirt(pte) (phys2kvirt(ppn2phys(pte_get_ppn(pte))))

#define pte_is_valid(pte) pte_get(pte, PTE_VALID_SHIFT)

#define ppn2phys(ppn) ((ppn) << PAGE_SHIFT)
#ifndef phys2ppn
#define phys2ppn(phys) ((phys) >> PAGE_SHIFT)
#endif
#define vpn2virt(vpn) ppn2phys(vpn)
#define virt2vpn(virt) phys2ppn(virt)

/*
 * The virtual memory area (VMA) structure, cornerstone of virtual memory, and
 * what is exported to other kernel subsystems (like buddy exported blocks)
 */
struct vma {
	uintptr_t vpn;
	size_t pagen;
	uintptr_t flags;
	struct node node;
};

/*
 * pt_alloc: allocates a page of a certain level (NORMAL, MEGA, etc.)
 *
 * Returns a physical address. Can be accessed (inside the kernel) by using the
 * phys2kvirt() macro
 *
 * NOTE: to be used for large allocations of kernel memory, most often though,
 * you'll want to use pt_salloc (see below)
 *
 */
uintptr_t pt_alloc(size_t level);

/*
 * pt_salloc: a simple "cursor allocator" for small allocations. It just
 * moves a cursor forward along a page. Does not support free'ing memory later!
 *
 * @size: number of bytes of the allocated region
 *
 * Returns a pointer to the allocated region, in kernel space
 *
 * FIXME: turn this into a proper slab allocator
 */
void *pt_salloc(size_t size);

/*
 * pt_free: frees a page allocated with pt_alloc()
 *
 * @phys: physical address of the page to be freed
 *
 * Returns 0 on success, -1 on failure
 *
 * NOTE: does not support freeing memory from pt_salloc (that's a FIXME)
 *
 */
int pt_free(uintptr_t phys);

/*
 * pt_vma_new: creates a new VMA and maps it
 *
 * @satp: address space to map the VMA into
 * @vpn: virtual page number to be mapped
 * @pagen: size of mapping, in number of pages of size PAGE_NORMAL
 * @flags: permissions and extras such as VMA_POPULATE
 *
 * @out: your VMA, which you should have allocated already and put on a linked
 * list, but that we initialized
 *
 * Returns 0 on success, -error on failure
 *
 */
ssize_t pt_vma_new(uintptr_t *satp, uintptr_t vpn, size_t pagen,
		   uintptr_t flags, struct vma *out);

/*
 * pt_vma_destroy: opposite of pt_vma_new: removes a VMA both from the linked
 * list it is on and removes all its mappings
 */
void pt_vma_destroy(uintptr_t *satp, struct vma *vma);

/*
 * pt_vma_map_page_at_vpn: maps a page at @vpn inside @vma, possibly using
 * @__ppn as the page's PPN (if VMA_IDENTITY is set), but typically asking
 * buddy for a PPN
 *
 * Returns the size, in pagen, of the mapped page. Will try to maximize the
 * size of the page. Returns -error on failure
 *
 * This function is important. It is used by the page fault handler, and also
 * by pt_vma_new
 */
ssize_t pt_vma_map_page_at_vpn(uintptr_t *satp, struct vma *vma, uintptr_t vpn,
			       uintptr_t __ppn);

/*
 * pt_vma_unmap_pages: almost like pt_vma_destroy, except that it does not
 * remove the VMA from the linked list (meaning that the page fault handler
 * will map it again if it is accessed again)
 */
void pt_vma_unmap_pages(uintptr_t *satp, struct vma *vma);

/*
 * vpn2vma: given @vpn, will return the @vma it is in or NULL if there is no
 * such VMA
 *
 * Used by the page fault handler to distinguish major page faults from minor
 * page faults
 *
 * FIXME: can be done more efficiently (some sorting?), but OK for now
 */
struct vma *vpn2vma(struct node *vmas_head, uintptr_t vpn);

void pt_flush_tlb(void);
void *phys2kvirt(uintptr_t phys);

/* Only used during initialization, you probably don't want to use these */
void pt_jump_to_high(void);
void pt_destroy_low_kvmas(void);
int pt_init(void);

uintptr_t pt_alloc_pt(void);
int pt_init_kvmas(size_t n, struct vma **_kvmas);
int pt_init_kvma(struct vma *out, struct vma **_kvmas);

void set_vma_protection();
void unset_vma_protection();

ssize_t pt_get_pte(uintptr_t *satp, uintptr_t vpn, uintptr_t *ptes,
		   uintptr_t **out);

struct vma *pt_alloc_vma(struct node *vmas_head);
int pt_init_vmas_head(struct node **vmas_head);

void *get_kvirt(uintptr_t *satp, void *user_virtual);

#endif
