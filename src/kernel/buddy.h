#include "types.h"
#include "util.h"

/* FIXME: move includes to .c instead!? */
#pragma once

struct buddy_layout {
	uintptr_t kelf_base;
	uintptr_t phys_base;
	uintptr_t phys_end; /* Also kstack_top */
	uintptr_t kstack_size;
};

struct block {
	size_t refcnt;
	uintptr_t order; /* 0 <= order <= BUDDY_MAX_ORDER */
	struct block *next;
};

void buddy_init(void);
void buddy_migrate(uintptr_t offset);

struct block *buddy_alloc(unsigned order);
int buddy_free(struct block *block);

struct block *ppn2block(uintptr_t ppn);
uintptr_t block2ppn(struct block *block);
