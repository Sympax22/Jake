// --- INCLUDES --- //
#include "types.h"
#include "util.h"
#include "buddy.h"
// --- INCLUDES --- //

// --- MACROS --- //
// --- Physical Macros --- //
#define PHYS_MEM_ORDER 25 /* 32 MB */
#define PHYS_MEM_SIZE (1UL << PHYS_MEM_ORDER)

#define PHYS_MEM_PAGE_ORDER 12
#define PHYS_MEM_PAGE_SIZE (1UL << PHYS_MEM_PAGE_ORDER)

#define PHYS_MEM_START 0x80000000
#define PHYS_MEM_END (PHYS_MEM_START + PHYS_MEM_SIZE)

#define PHYS_MEM_KERNEL_STACK_SIZE (1 * PHYS_MEM_PAGE_SIZE)
// --- Physical Macros --- //

// --- Buddy-specific Macros --- //
#define BUDDY_MAX_ORDER (PHYS_MEM_ORDER - PHYS_MEM_PAGE_ORDER)

/* | 0x80000000                        |                        |             |
 * | .text segments... .text of buddy  | buddy's .data and .bss | -->         |
 * |                                   |                     <-- kernel stack |
 */

/* __ppn is buddy's internal ppn, with __ppn == 0 for PHYS_MEM_START */
// physical address to physical ppn
#define phys2ppn(phys) ((phys) >> PHYS_MEM_PAGE_ORDER) 					  
// physical address to virtual  ppn
#define phys2__ppn(phys) (((phys)-PHYS_MEM_START) >> PHYS_MEM_PAGE_ORDER) 

// virtual  ppn to physical ppn
#define __ppn2ppn(__ppn) ((__ppn) + phys2ppn(PHYS_MEM_START))     
// physical ppn to virtual  ppn        
#define ppn2__ppn(ppn) ((ppn)-phys2ppn(PHYS_MEM_START))                   

// virtual ppn to block
#define __ppn2block(__ppn) ((__ppn) + buddy_blocks)   
// block       to virtual ppn                    
#define block2__ppn(block) ((block)-buddy_blocks)                         

// physical address to block
#define phys2block(phys) __ppn2block(phys2__ppn(phys))                    

#define is_list_empty(order) (!(uintptr_t)buddy_free_lists[order])
#define is_block_free(block) (!(block->refcnt))

/*
 * TODO: start by implementing block2buddy(block, order), where order is the
 * order of block. Use __ppn2block and block2__ppn
 */
#define block2buddy(block, order) (__ppn2block(block2__ppn(block) ^ (1 << order))) // ERROR?
// --- Buddy-specific Macros --- //
// --- MACROS --- //

// --- STRUCTURES --- //
/**
 * Array of linked lists
 * One linked list per order
 * Contains only free blocks
 */
static struct block *buddy_free_lists[BUDDY_MAX_ORDER + 1];	

/**
 * Linear array of block structures 
 * Is to be used as a stack
 * DO NOT index directly
 * Use push and pop from buddy_free_lists
 */
static struct block buddy_blocks[1UL << BUDDY_MAX_ORDER];
struct buddy_layout layout;
extern uintptr_t phys_base;
// --- STRUCTURES --- //

// --- FUNCTIONS --- //
static void buddy_print_status();
static struct block *buddy_pop(unsigned order);
static struct block *buddy_remove(struct block *block, unsigned order);
static void buddy_push(struct block *block, unsigned order);
static int __buddy_find_smallest_free_order(unsigned order);
static int __buddy_split(unsigned order);
static int buddy_split(unsigned smallest_free_order, unsigned desired_order);
static struct block *__buddy_merge(struct block *block, struct block *buddy);
static void __buddy_try_merge(struct block *block);
int buddy_free(struct block *block);
struct block *buddy_alloc(unsigned order);
struct block *ppn2block(uintptr_t ppn);
uintptr_t block2ppn(struct block *block);
static void buddy_layout_init();
static void __buddy_init(void);
static void buddy_reset(void);
void buddy_migrate(uintptr_t offset);
void buddy_init(void);
// --- FUNCTIONS --- //

// --- CODE --- //
/**
 * @brief code starts here
 * Not written by us. Uses __buddy_init() and buddy_layout_init()
 */
void buddy_init(void)
{
	__buddy_init();

	buddy_layout_init();

	printstr("--- buddy layout ---\n");
	printdbg("PHYS_MEM_START       : ", (void *)layout.kelf_base);
	printdbg("phys_base            : ", (void *)layout.phys_base);
	printdbg("phys_end             : ", (void *)layout.phys_end);
	printdbg("PHYS_MEM_END         : ", (void *)PHYS_MEM_END);
	printdbg("kstack_size          : ", (void *)layout.kstack_size);

	printstr("--- buddy meta data ---\n");
	printdbg("buddy_free_lists base: ", (void *)buddy_free_lists);
	printdbg("buddy_free_lists end : ",
		 (char *)buddy_free_lists + sizeof(buddy_free_lists));
	printdbg("buddy_blocks base    : ", (void *)buddy_blocks);
	printdbg("buddy_blocks end     : ",
		 (char *)buddy_blocks + sizeof(buddy_blocks));
}

/**
 * @brief initalizes buddy? 
 * Not written by us. Uses buddy_free() 
 */
static void __buddy_init(void)
{
	phys_base = (uintptr_t)&phys_base;

	struct block block = { .refcnt = 1, .order = 0, .next = NULL };

	for (size_t i = 0; i < ARRAY_SIZE(buddy_blocks); i++) 
	{
		buddy_blocks[i] = block;
	}
	
	for (uintptr_t phys = phys_base;
	     phys < PHYS_MEM_END - PHYS_MEM_KERNEL_STACK_SIZE;
	     phys += PHYS_MEM_PAGE_SIZE) 
	{
		// TODO: fix buddy_free to make initialization work 
		// Fixed!
		buddy_free(phys2block(phys));
	}
}

/**
 * @brief Attempts to free the given block. This is where the problems start...
 * Written by us. Uses buddy_push() and __buddy_try_merge()
 * @param block 
 * @return int (free worked:0 or not:-1)
 */
int buddy_free(struct block *block)
{
	switch (block->refcnt) 
	{
	case 0:
		return -1; /* Double free */

	case 1:
		/*
		 * TODO: what should happen here? Hint: use buddy_push. Hint:
		 * also use __buddy_try_merge after you've implemented
		 * __buddy_merge 
		 * Fixed!
		 */
		buddy_push(block, block->order);
		__buddy_try_merge(block);
		return 0;

	default:
		/* TODO: and here? */
		// Update refcnt, one less process uses it 
		// Fixed!
		block->refcnt -= 1;
		return 0;
	}
	return -1;
}

/**
 * @brief pushes the given block on the buddy_free_lists[order]
 * Written by us. Should be correct? 
 * @param block 
 * @param order 
 */
static void buddy_push(struct block *block, unsigned order)
{
	/*
	 * TODO: what should happen here? Hint: at least 2 lines. Hint: insert
	 * the block in front of the linked list it belongs to
	 * Fixed!
	 */
	// Current block should become the new head, so it has to point to the old head
	block->next = buddy_free_lists[order];
	block->refcnt = 0;
	// New head is current block
	buddy_free_lists[order] = block;
}

/**
 * @brief checks the condition to do a merge, and merges until it can't not more
 * Not written by us, uses __buddy_merge() and itself! RECURSION!
 * @param block 
 */
static void __buddy_try_merge(struct block *block)
{
	if (block->order == BUDDY_MAX_ORDER) return;

	// Initialize the buddy of the spot being freed in the parking lot
	struct block *buddy = block2buddy(block, block->order);

	if (block->order != buddy->order) return;
	
	if (is_block_free(block) && is_block_free(buddy)) 
	{
		block = __buddy_merge(block, buddy);
		__buddy_try_merge(block);
	}
}

/**
 * @brief merges two free blocks into one of bigger order
 * Written by us, probably does not work correctly 
 * @param block 
 * @param buddy 
 * @return struct block* 
 */
static struct block *__buddy_merge(struct block *block, struct block *buddy)
{
	/*
	 * TODO: implement the actual merging here. Think about which of these
	 * blocks will become "the larger one". Make use of buddy_remove and
	 * buddy_pop. Return the merged block
	 * Fixed!
	 */
	if(buddy < block) return __buddy_merge(buddy, block);

	unsigned int order = block->order;

	buddy_remove(buddy, order);
	buddy_remove(block, order);

	block->order += 1;

	buddy_push(block, block->order);
	return block;
}

/**
 * @brief removes a block from the given buddy_free_lists[order]
 * Not written by us, uses buddy_pop()
 * @param block 
 * @param order 
 * @return struct block* 
 */
static struct block *buddy_remove(struct block *block, unsigned order)
{
	if (is_list_empty(order)) return NULL;

	if (buddy_free_lists[order] != block) 
	{
		struct block *head = buddy_free_lists[order];
		struct block *tail = head;
		struct block *prev = NULL;

		while (tail->next) 
		{
			if (tail->next == block) prev = tail;
			tail = tail->next;
		}

		if (!prev) return NULL; /* Could not find block */

		buddy_free_lists[order] = block;
		prev->next = NULL;
		tail->next = head;
	}
	return buddy_pop(order);
}

/**
 * @brief takes off a block from the given buddy_free_lists[order]
 * Written by us, probably works correctly?
 * @param order 
 * @return struct block* 
 */
static struct block *buddy_pop(unsigned order)
{
	if (is_list_empty(order)) return NULL;

	// block is head of the list that we want to pop
	struct block *block = buddy_free_lists[order];

	/* TODO: What should happen here? */
	// we now need to update the head of the list before returning
	buddy_free_lists[order] = block->next;
	block->refcnt += 1;
	block->next = NULL;

	return block;
}

/**
 * @brief Allocates a block of order order. Might not give aligned pages
 * Written by us
 * @param order 
 * @return struct block* 
 */
struct block *buddy_alloc(unsigned order)
{
	/* Order too large? */
	if (order > BUDDY_MAX_ORDER) return NULL;

	/* TODO: call __buddy_find_smallest_free_order here */
	/*
	 * TODO: think about what __buddy_find_smallest_free_order might
	 * return and what should happen in each case (return early upon a
	 * failure)
	 */
	int smallest_order = __buddy_find_smallest_free_order(order);
	
	if(smallest_order < 0) return NULL;
	if(buddy_split(smallest_order, order) != 0)	return NULL;

	return buddy_pop(order);
}

/**
 * @brief 
 * Not written by us
 * @param smallest_free_order 
 * @param desired_order 
 * @return int 
 */
static int buddy_split(unsigned smallest_free_order, unsigned desired_order)
{
	/* Will do nothing if smallest_free and desired are equal */
	for (unsigned i = smallest_free_order; i > desired_order; i--) 
	{
		int ret = __buddy_split(i);
		if (ret) return ret;
	}
	return 0;
}

/**
 * @brief 
 * Written by us
 * @param order 
 * @return int 
 */
static int __buddy_split(unsigned order)
{
	if (order == 0) return -1;

	struct block *block = buddy_pop(order);

	if (!block) return -2;

	/* TODO: implement the actual splitting here */
	// Fixed!
	block->order -= 1;
	buddy_push(block, block->order);
	struct block* buddy = block2buddy(block, block->order);
	buddy_push(buddy, block->order);

	return 0;
}

/**
 * @brief 
 * Not written by us
 * @param order 
 * @return int 
 */
static int __buddy_find_smallest_free_order(unsigned order)
{
	for (unsigned i = order; i <= BUDDY_MAX_ORDER; i++) 
		if (!is_list_empty(i)) return i;

	return -1;
}

/**
 * @brief initalizes the layout?
 * Not written by us.
 */
static void buddy_layout_init()
{
	layout.kelf_base = PHYS_MEM_START;
	layout.phys_base = phys_base; /* This one moves, as the kernel grows */
	layout.phys_end = PHYS_MEM_END - PHYS_MEM_KERNEL_STACK_SIZE;
	layout.kstack_size = PHYS_MEM_KERNEL_STACK_SIZE;
}

static void buddy_print_status()
{
	for (size_t i = 0; i < ARRAY_SIZE(buddy_free_lists); i++) 
	{
		size_t free_blocks = 0;

		printptr((void *)i, '\n');

		if (!is_list_empty(i)) 
		{
			struct block *block = buddy_free_lists[i];
			do 
			{
				free_blocks++;
			} while (block->next);
		}

		printptr((void *)free_blocks, '\n');
		printstr("----------------\n");
	}
}

struct block *ppn2block(uintptr_t ppn)
{
	if (ppn < phys2ppn(PHYS_MEM_START) || ppn >= phys2ppn(PHYS_MEM_END)) 
		return NULL;
	else 
		return __ppn2block(ppn2__ppn(ppn));
}

uintptr_t block2ppn(struct block *block)
{
	return __ppn2ppn(block2__ppn(block));
}

static void buddy_reset(void)
{
	memset(buddy_free_lists, 0,
	       sizeof(struct block *) * ARRAY_SIZE(buddy_free_lists));
	memset(buddy_blocks, 0,
	       sizeof(struct block) * ARRAY_SIZE(buddy_blocks));
	__buddy_init();
}

void buddy_migrate(uintptr_t offset)
{
	for (size_t i = 0; i < ARRAY_SIZE(buddy_free_lists); i++) 
	{
		if (!buddy_free_lists[i]) continue;

		buddy_free_lists[i] =
			(struct block *)((char *)buddy_free_lists[i] + offset);
	}

	for (size_t i = 0; i < ARRAY_SIZE(buddy_blocks); i++) 
	{
		struct block *block = &buddy_blocks[i];
		if (block->next) 
		{
			block->next =
				(struct block *)((char *)block->next + offset);
		}
	}
}
// --- CODE --- //