% BUDDY(3) COMSEC | Computer Engineering '22: Homework Guide

# NAME
buddy - Manage your physical memory

# DEADLINE
Friday, April 8, 2022 @ 11:59 (noon) (see Moodle)

# SOURCES
`./src/kernel/buddy.c`

# HANDING IN
See `general(0)`

# COMMANDS
See `general(0)`

# DESCRIPTION
Last week, we showed you how the kernel bootstraps itself in assembly. It now has a stack, and we are ready to jump to `main`.

The first thing your kernel is going to do is manage what might be its most precious resource: physical memory. To this end, you are going to implement a memory allocator. The algorithm that we will use is called buddy, which is also the algorithm used by Linux, although our version will be much simpler.

A high-level explanation of buddy was given during the lectures and is also easy to find online. Make sure you understand the algorithm before reading on, because otherwise you will not be able to understand the implementation details that follow.

First, let's have a look at the interface exported by your allocator (see `buddy.h`):

## struct block *buddy_alloc(unsigned order)

Allocates a block of the specified order and returns it to the caller. Returns `NULL` if no such block is available

## int buddy_free(struct block *block)

Frees the block passed as argument. Returns 0 on success and `-ERROR` otherwise

## struct block *ppn2block(uintptr_t ppn)

Takes a physical page number `ppn` (or page frame number) and returns a pointer to its block structure

## uintptr_t block2ppn(struct block *block)

Takes a pointer to a block and returns its `ppn`

The first two functions should be self-explanatory. The buddy allocator uses blocks to describe physical memory. A `block` is a contiguous region of physical memory. The size of a block is determined by its order and equals 4 KiB * 2^`order`. Depending on the context, we might also use the term *page frame* to refer to a contiguous region of physical memory, in particular if we are talking about both virtual and physical memory.

Although the allocator operates on block structures (i.e., `struct block`), it may sometimes be necessary (as you will see in later assignments) to convert a block structure into its `ppn` and vice versa. That's why need `ppn2block` and `block2ppn`, which we have already implemented for you.

## Important data structures

There are two important data structures that you will be using: `buddy_free_lists` and `buddy_blocks`. The former is a list of linked lists (!), one linked list per order, that contains only free blocks. The latter is a linear array of block structures. You should not index `buddy_blocks` directly. Instead, you will be pushing blocks to and popping blocks from `buddy_free_lists`.

Finally, let's have a look at the block structure:

```
struct block {
	size_t refcnt;
	uintptr_t order; /* 0 <= order <= BUDDY_MAX_ORDER */
	struct block *next;
};
```

The `refcnt` field is a reference count. It determines whether a block is free: if there is no one (i.e., no process or kernel) who holds a reference to this block, the block may be freed. Otherwise, upon a free, the block may not be freed, and we only decrease its reference count.

The `order` field denotes the order of the block, as explained above.

The `next` field is a pointer to the next block, if the current block is on a linked list. It should be `NULL` if the current block is at the end of the list or not on the list. 

# TASKS

Take care of all the TODOs in `buddy.c`. That is, implement `buddy_alloc` and `buddy_free`. Please note that

1. `buddy_alloc` depends on `buddy_split`, `__buddy_find_smallest_free_order`, and `buddy_pop`. We have implemented `buddy_split` for you, but it depends on `__buddy_split` which you should implement.
2. `buddy_free` depends on `buddy_push` and `__buddy_try_merge`. We have implemented `__buddy_try_merge` for you, but it depends on `__buddy_merge` which you should implement.

Start by implementing the `block2buddy(block, order)` macro, which is already used in `__buddy_try_merge` but which you will also need for splitting. Make use of `__ppn2block` and `block2__ppn`. Try to understand what the other macros are used for. After you've implemented `block2buddy`, the compiler warning you get the first time you compile your kernel should disappear.

Then move on to fix `__buddy_init` which depends on `buddy_free`. Implement `buddy_free` and you should see sensible output from the first test `alloc_max_order`. You can now start working on the remaining TODOs.

The trickiest parts will be implementing `__buddy_split` and `__buddy_merge`. Think carefully about what should happen when merging and splitting, and in which order. Make use of `buddy_pop` (when splitting), `buddy_push` (when splitting and merging), and `buddy_remove` (when merging).

**You should not need more than 30 lines of code.**

# TESTS

The following tests will determine your grade:

## alloc_max_order (1/6)

Allocates a single block of order `BUDDY_MAX_ORDER`

## free_max_order (1/6)

Frees the block obtained above

## alloc_order_zero (1/6)

Allocates a single page frame of order zero. This will involve splitting several blocks

## free_order_zero (1/6)

Frees the frame obtained above. This will involve merging several blocks

## random_alloc_free (2/6)

Performs a number of random allocations of different orders, intertwined with frees. This is a real stress test

# COLOPHON
This is the second homework assignment of Computer Engineering '22.

# AUTHORS
Finn de Ridder
