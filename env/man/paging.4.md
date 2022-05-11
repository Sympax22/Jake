% PAGING(4) COMSEC | Computer Engineering '22: Homework Guide
% Finn de Ridder

# NAME
paging - Manage your virtual memory

# DEADLINE
Friday, May 13, 2022 @ 11:59 (noon) (see Moodle)

# SOURCES
./src/kernel/pt.c

./src/kernel/exception.c

./src/kernel/main.c (no TODOs here, but you may want to check out what changed)

# HANDING IN
See **general(0)**

# COMMANDS
See **general(0)**

# DESCRIPTION
Last week you worked on buddy, your physical memory allocator. You are now able to allocate and free physical memory for use by the kernel itself. Before we are able to expose memory to applications however, we have to create the abstraction of virtual memory. Please refer to the lectures for a high-level explanation of what virtual memory is and why almost all operating systems use it. (This page will give you the low-level explanation.)

## Disclaimer
**At the time of writing, you have only had one lecture about virtual memory. The other lecture will be given after the holidays. This assignment, however, is about both lectures. For this reason, do not expect (and we do not expect) to understand everything that you will find on this man page. And don't worry: the assignment will be explained in full glory on the Friday after the holidays. However, try to get started, have a look at the code, and give the TODOs that involve low-level page table management a try, those you should be able to do.**

## Page tables
The mapping from virtual to physical memory is defined by a page table. The actual translation is performed in hardware by the memory management unit (MMU). Because the MMU is part of the processor, it should be no surprise that the format of the page table is architecture dependent.

The page table format that we will use is RISC-V's Sv48 (see p. 77 of Volume II: RISC-V Privileged Architectures). Sv48 means that our (virtual) address spaces will be 48 bits large or 256 TiB, much more than the 32 MiB of physical memory that our virtual machine is equipped with. Sv48 is similar (but not identical!) to the page table format of x86-64, which most of your laptops have.

## Jump high
At the end of this lab, you will "jump high" which means that you will be accessing memory beyond the 32 MiB boundary of reality and somewhere in a large imaginary 256 TiB (virtual) address space. Of course, this will only work if you have set up your page tables correctly, because only then the MMU will be able to translate your high addresses to low addresses somewhere in (real) physical memory.

The function that jumps high is called `pt_jump_to_high`. It is called in `main.c` instead of `pt.c` for reasons that will become clear later. What it does may not be apparent immediately, but don't worry, soon you will understand and besides, it does not contain any TODOs anyway.

We will only jump high after we have turned on the MMU by calling `ksatp_enable_paging`. As soon as we call `ksatp_enable_paging`, every memory address will be interpreted as being virtual and trigger a page table walk. This means that if there are no page tables, or if they do not conform to Sv48, either we crash directly or a page fault is triggered, which transfers the control flow to `./src/kernel/exception.c`. A crash is more likely though, and you will probably spend most of your time trying to get past `ksatp_enable_paging`.

## Low-level page table management
It should be clear by now that this lab will be all about page table management: creating page tables, updating page tables, removing page tables, etc. What makes page table management tricky is the tree-like structure of the page table. The Sv48 page table has four levels, which means that in order to update just one page table entry (PTE) in the lowest level, we need to "walk the page table" in software, starting from the top of the page table (tree), all the way down to the level at which we would like to change something. Most of the functions that you will find in `pt.c` do something related to this page table walk in software.

The most important function of this lab is probably:

## int pt_set_pte_leaf(uintptr_t *satp, uintptr_t vpn, uintptr_t pte, size_t depth)

This function updates a PTE `(uintptr_t pte)` in a given page table `(uintptr_t *satp)`. A PTE is 64 bits and its format is given in Fig. 4.23 on p. 78 of the RISC-V manual mentioned earlier. There you will find that a PTE contains the following fields:

* A page frame number (PPN) of 44 bits
* The permission bits X, W, R for execute, write, and read permission, respectively
* A validity bit V

It actually contains more information, but the other bits are not relevant for now. The PPN and permission bits should be self-explanatory. The PPN is basically the physical address that this PTE points to, but without the page offset, which is not translated (i.e., there is no such thing as a "virtual page offset"). The permission bits determine what kind of access is allowed to the page frame referenced by the PPN.

What is the validity bit for? If the V-bit is not set, the MMU will complain and cause a page fault. The V-bit is used to distinguish between three kinds of PTEs (think of the page table as a tree):

1. **A leaf or final PTE:** the PTE where a page table walk (for some virtual address) ends. The PPN of a leaf PTE points to a page in physical memory. The virtual address is mapped to that physical page
2. **A non-leaf PTE or intermediate PTE:** a PTE whose PPN points to another page table, which means we need to continue walking our page tables until we hit a leaf PTE
3. **An empty PTE:** nowhere to go from here!

The V-bit is used to distinguish leaf and non-leaf PTEs from empty PTEs (V-bit not set), whereas the permission bit R (for read permission) is used to distinguish between leaf (R-bit set) and non-leaf PTEs (R-bit not set).

Let's return to `pt_set_pte_leaf` and discuss the other arguments. As mentioned before, the to-be-updated page table is specified using the `satp` argument, which is a pointer to the so-called address translation and protection register `satp` (see p. 68 of the RISC-V manual). The pointer does not actually point to the register, but rather to a value (in memory) that is formatted like the register, and which at some point may be written to the register (to make those page tables active). The format of the `satp` register, then, is more or less like that of a PTE except that it only contains a PPN and no permission bits. The PPN stored in the `satp` register is the address of root page table of a page table tree and therefore the starting point of every page table walk.

To update our PTE (remember that that's what `pte_set_pte_leaf` is all about) we need to locate it in the page table tree, which we cannot do with just the `satp` and PTE itself. If you think of a page table tree as having two dimensions, a width and a depth, then we need exactly two more pieces of information to locate a PTE. As you have probably guessed, the `depth` argument determines the depth and ranges from 0 up to and including 4, with a depth of 0 being `satp` itself, 1 the table pointed to by `satp`, etc. The horizontal axis is determined by the virtual page number `vpn`, which at each level is used to index the page table at that level to find there either the final PTE or simply a pointer to the next table.

There's a big comment in the beginning of `pt.c` that also explains `depth` and other important terms for this lab such as `level` and `pagen`.

## uintptr_t *__pt_next_pte(size_t depth, uintptr_t vpn, uintptr_t *ptes)

Although `pt_set_pte_leaf` is probably the most important function of this lab, it heavily relies on `__pt_next_pte` which is the function that `pt_set_pte_leaf` uses to take a single step in a page table walk. Based on the first virtual memory lecture, you should be able to implement both of them.

## From low-level page table management to virtual memory areas (VMAs)
In theory, page tables are all you need to set up virtual memory, or rather, page tables are all that the MMU needs to function properly. However, for virtual memory to actually be useful, we may want to add a few abstraction layers on top of those awkward page tables. As will be explained during the second virtual memory lecture, that abstraction layer is the abstraction of virtual memory areas (VMAs). In short, a VMA (or VMA structure) simply describes a range of (valid) contiguous virtual memory, consisting of multiple pages that all have the same permissions.

The power of VMAs is that they enable us to create rich virtual address spaces (with stacks, heaps, shared libraries, and what not) without having to "map" each page to a page frame in physical memory (remember that in `jake` we only have 32 MiB!). In particular, VMAs allow us to allocate physical memory lazily, on demand, or just in time: we allocate physical memory and set up the page table only at time of (first) access, not at time of map (unless **VMA_POPULATE** is set, see below). In this way, we do not waste physical memory on virtual memory that is never used. **Important: we do not map the entire VMA as soon as one of its pages is accessed, but only map that single page inside the VMA.**

The VMA structure is defined in `pt.h` and is actually very simple:

```
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
```
The `vpn` is the start of the virtual memory area, it is the VPN of the first page of the VMA. The size of the VMA (`pagen`) is given in terms of 4 KiB pages, so if `pagen` is 2, the VMA starting at `vpn` is 8 KiB. The `flags` field contains information about the VMA's permissions and its type, in particular, is this a "demand paged" VMA or not? By default, all VMAs use demand paging, but sometimes we want to back our VMA by physical memory immediately; in that case we set the **VMA_POPULATE** flag. The `node` field is used to insert the VMA structure on a doubly-linked list, which is like the singly-linked list that you used for buddy, but with both a `prev` and a `next` pointer. You will not be interacting with it much.

## VMA_IDENTITY and bootstrapping into virtual memory
Finally, there is also a **VMA_IDENTITY** flag, which is a bit special. Normally, to back a VMA with physical memory, we ask our buddy allocator to allocate some for us. However, the memory used by the buddy allocator itself, such as the infamous `buddy_blocks` array, or the memory used by the kernel, or the kernel stack, are all marked as "allocated" for good reason. Does this mean that we cannot map (in virtual memory) those parts of physical memory? If so, then that would be a problem: we will not be able to access them once the MMU is turned on.

This is where **VMA_IDENTITY** comes into play: it allows us to create a VMA that maps to a physical address that we decide on (see `__ppn` of `__pt_vma_new` in `pt.c`) and bypass the buddy allocator.

# TASKS
Take care of all the TODOs in `pt.c` and `exception.c`. First, you will work on low-level page table management, you will then play with virtual memory areas (VMAs) and demand paging, and finally implement the missing two lines in the page fault hander.

1. Start with `pt_set_pte_leaf`, the page table walker. It depends on `__pt_next_pte` and `pt_set_pte_nonleaf`, the former of which you will have to implement yourself.

(After the second virtual memory lecture...)

2. Continue by implementing the functions that have to do with VMAs, in particular `__pt_vma_new`, which depends on `pt_vma_new_page_at_vpn`, which itself depends on (eventually) `__pt_page_pte_set_perm` and `__pt_page_needs_pte`.

3. Finally, fix the page fault handler in `./src/kernel/exception.c`.

You will need about 90 lines of code.

# WARNING
Due to the nature of this lab, we are only able to test your implementation after enabling paging first in `pt_init`. However, `pt_init` depends on a more or less functional implementation of virtual memory...

As a consequence, for this lab our tests will not be able to guide you: they will only run if `pt_init` returns without crashing, which again, will only be after you have implemented most of the above correctly. In other words, the tests are there to see if you got the details right, but most of the debugging you will have to do yourself.

Be prepared for continuous crashing!

# TIPS
To make debugging a bit easier, make use of `pt_pte_print` to print a PTE and `pt_print` to print your page tables. The latter may not work immediately, however.

Finally, compared to `buddy(3)` the source code for this lab contains much more comments, read them!

# TESTS
The following tests will determine your grade (the points will be normalized to ETH grades):

## map_one_page (1/9)

Maps a single page (with VMA_POPULATE), checks whether there's a PTE at the right location

## map_several_pages (1/9)

Maps 13 pages (without VMA_POPULATE), checks whether there's a VMA

## access_demand_several_pages (1/9)

Tries to access the 13 pages that get mapped by the previous test. This should trigger demand paging (so your page fault handler should work)

## map_mega_page (1/9)

Maps 512 pages at an aligned virtual address (with VMA_POPULATE), checks whether your PTE maps a huge page

## map_mega_normal_pages (1/9)

Maps 515 pages (without VMA_POPULATE), should get a huge page and a few smaller ones

## access_demand_mega_normal_pages (1/9)

Tries to write to the huge page mapped by the previous test. This should trigger demand paging

## map_populate_mega_normal_pages (1/9)

Maps 634 pages (with VMA_POPULATE)

## access_mega_normal_pages (1/9)

Tries to write to somewhere in the region mapped by the previous test

## map_several_vmas (1/9)

Tries to create two VMAs and checks their PTEs

# COLOPHON
This is the third homework assignment of Computer Engineering '22.
