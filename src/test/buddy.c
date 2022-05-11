#include "../kernel/buddy.c"
#include "test.h"

enum tests {
	alloc_max_order,
	free_max_order,
	alloc_order_zero,
	free_order_zero,
	random_alloc_free,
	__success
};

static const struct test ts[] = {
	[alloc_max_order] = { "alloc_max_order", 1 },
	[free_max_order] = { "free_max_order", 1 },
	[alloc_order_zero] = { "alloc_order_zero", 1 },
	[free_order_zero] = { "free_order_zero", 1 },
	[random_alloc_free] = { "random_alloc_free", 2 }
};

struct buddy_state {
	ssize_t free_lists_lens[BUDDY_MAX_ORDER + 1];
	size_t num_free_pages;
	ssize_t max_free_order;

	void (*update)(struct buddy_state *);
	int (*is_equal)(struct buddy_state *, struct buddy_state *);
};

#define BUDDY_TEST_RANDOM_NUMBER_OF_TESTS 64
#define BUDDY_TEST_RANDOM_MAX_DEPTH 128
#define BUDDY_TEST_FREE_PROBABILITY 3
struct buddy_test_machine_random {
	unsigned seed;
	size_t depth;
	int allocs[BUDDY_TEST_RANDOM_MAX_DEPTH];
	int frees[BUDDY_TEST_RANDOM_MAX_DEPTH];
};

struct buddy_test_machine {
	struct block *block;
	struct block shadow_block;

	struct buddy_state state;
	struct buddy_state shadow_state;

	struct buddy_test_machine_random btmr;
};

static void buddy_state_update_free_lists_lens(struct buddy_state *in)
{
	for (size_t i = 0; i < ARRAY_SIZE(in->free_lists_lens); i++) {
		size_t count = 0;

		if (!is_list_empty(i)) {
			struct block *block = buddy_free_lists[i];
			count = 1;

			while (block->next) {
				block = block->next;
				count++;
			}
		}

		in->free_lists_lens[i] = count;
	}
}

static void buddy_state_update_num_free_pages(struct buddy_state *in)
{
	size_t num_free_pages = 0;
	for (size_t i = 0; i < ARRAY_SIZE(in->free_lists_lens); i++) {
		num_free_pages += in->free_lists_lens[i] * (1 << i);
	}

	in->num_free_pages = num_free_pages;
}

static void buddy_state_update_max_free_order(struct buddy_state *in)
{
	for (ssize_t i = BUDDY_MAX_ORDER; i >= 0; i--) {
		if (in->free_lists_lens[i]) {
			in->max_free_order = i;
			return;
		}
	}

	in->max_free_order = -1;
}

static void buddy_state_update(struct buddy_state *in)
{
	buddy_state_update_free_lists_lens(in);
	buddy_state_update_num_free_pages(in);
	buddy_state_update_max_free_order(in);
}

static int buddy_state_is_equal(struct buddy_state *in,
				struct buddy_state *state)
{
#define BUDDY_STATE_WILDCARD -1
	for (size_t i = 0; i < ARRAY_SIZE(in->free_lists_lens); i++) {
		if (state->free_lists_lens[i] == BUDDY_STATE_WILDCARD) {
			continue;
		} else if (in->free_lists_lens[i] !=
			   state->free_lists_lens[i]) {
			return 0;
		}
	}

	/*
	 * NOTE: we're not checking num_free_pages and max_free_order, because
	 * the check above implies it
	 */

	return 1;
}

/* Bit disgusting this function */
static void buddy_block_print_debug(struct block *in, struct block *shadow)
{
	int x, y;
	const void *args[] = { &shadow->order, &in->order };

	printstr("\t-------+------------+------------\n");
	printstr("\t       |   expected |        got \n");
	printstr("\t-------+------------+------------\n");

	printf("\t order |       %l |       %l", args);

	if (shadow->order != in->order) {
		printstr(" <--\n");
	} else {
		printstr(" \n");
	}

	x = is_block_free(shadow);
	y = is_block_free(in);

	args[0] = &x;
	args[1] = &y;

	printf("\t free? |       %d |       %d", args);

	if (x != y) {
		printstr(" <--\n");
	} else {
		printstr(" \n");
	}

	args[0] = &shadow->next;
	args[1] = &in->next;

	printf("\t  next | %p | %p", args);

	if (shadow->next != in->next) {
		printstr(" <--\n");
	} else {
		printstr(" \n");
	}
}

static void buddy_state_print_debug(struct buddy_state *in,
				    struct buddy_state *shadow)
{
	printstr("\t-----------+----------+------\n");
	printstr("\t     order | expected |  got \n");
	printstr("\t-----------+----------+------\n");
	for (size_t i = 0; i < ARRAY_SIZE(in->free_lists_lens); i++) {
		ssize_t got = in->free_lists_lens[i];
		ssize_t expected = shadow->free_lists_lens[i];

		const void *args[] = { &i, &expected, &got };
		printf("\t      %l |     %l | %l", args);

		if (got != expected) {
			printstr(" <--\n");
		} else {
			printstr(" \n");
		}
	}

	const void *args[] = { &shadow->num_free_pages, &in->num_free_pages };
	printstr("\t-----------+----------+------\n");
	printstr("\tfree pages | expected |  got \n");
	printstr("\t-----------+----------+------\n");
	printf("\t           |     %l | %l\n", args);
}

static void buddy_state_init(struct buddy_state *in)
{
	in->update = buddy_state_update;
	in->update(in);

	in->is_equal = buddy_state_is_equal;
}

static int buddy_block_is_equal(struct block *in, struct block *shadow)
{
	if (!in && shadow) return 0;
	if (!in && !shadow) return 1; /* valid OOM */
	if (in->order != shadow->order) return 0;
	if (is_block_free(in) != is_block_free(shadow)) return 0;
	if (in->next != shadow->next) return 0;
	return 1;
}

static int buddy_test_machine_random_init(struct buddy_test_machine_random *in,
					  struct buddy_state state,
					  unsigned seed)
{
	size_t i, j;
	unsigned r;
	int order;

	buddy_reset();
	state.update(&state);

	for (i = 0; i < BUDDY_TEST_RANDOM_MAX_DEPTH; i++) {
		/*
		 * NOTE: not using state.update() because we're manipulating it
		 * manually, working on a copy 
		 */
		buddy_state_update_max_free_order(&state);

		if (state.max_free_order < 0) break; /* OOM */

		j = 0;

		do {
			r = rand(rand(rand(seed) + i) + j);
			order = r % (state.max_free_order + 1);
			j++;
		} while (!state.free_lists_lens[order]);

		in->allocs[i] = order;

		state.free_lists_lens[order]--;

		in->frees[i] = !(r % BUDDY_TEST_FREE_PROBABILITY);

		if (in->frees[i]) state.free_lists_lens[order]++;
	}

	in->seed = seed;
	in->depth = i;

	return 0;
}

/*
 * FIXME: for better tests, implement support for delayed frees. This
 * requires us to keep track of which allocations are interdependent. Need to
 * distinguish between allocation that causes split and that does not (should
 * be easy to determine from state). Then, only if all interdependent allocs
 * are freed, we may expect the state we had before this "chain".
 */
static int buddy_test_machine_random_do(struct buddy_test_machine *btm)
{
	struct buddy_state *state = &btm->state;
	struct buddy_state *shadow_state = &btm->shadow_state;

	struct block *block = btm->block;
	struct block *shadow_block = &btm->shadow_block;

	/* We'll only be checking allocated block, so this is what we expect */
	shadow_block->refcnt = 1;
	shadow_block->next = NULL;

	struct buddy_test_machine_random *btmr = &btm->btmr;

	buddy_reset();

	/* Do the allocations, freeing some */
	for (size_t i = 0; i < btmr->depth; i++) {
		block = buddy_alloc(btmr->allocs[i]);
		shadow_block->order = btmr->allocs[i];

		if (!block) return -1;

		if (!buddy_block_is_equal(block, shadow_block)) return -2;

		if (btmr->frees[i]) buddy_free(block);
	}

	state->update(state);
	*shadow_state = *state;
	buddy_reset();

	/* Only do the allocations: state should be the same as above */
	for (size_t i = 0; i < btmr->depth; i++) {
		if (!btmr->frees[i]) {
			block = buddy_alloc(btmr->allocs[i]);
			shadow_block->order = btmr->allocs[i];

			if (!block) return -1;

			if (!buddy_block_is_equal(block, shadow_block))
				return -2;
		}
	}

	state->update(state);

	if (!buddy_state_is_equal(state, shadow_state)) return -3;

	if (!buddy_block_is_equal(block, shadow_block)) {
		printstr("STS!\n");
	}

	return 0;
}

#define update_args()                                                          \
	do {                                                                   \
		args[0] = ts[tm.t].name;                                       \
	} while (0)

int buddy_test(void)
{
	int ret;

	struct buddy_state tmp;

	struct test_machine tm = { 0 };
	struct buddy_test_machine btm = { 0 };

	buddy_reset();

	buddy_state_init(&btm.state);
	buddy_state_init(&btm.shadow_state);

	const void *args[2];

	for (tm.t = 0; tm.t < __success; tm.t++) {
		switch (tm.t) {
		case alloc_max_order:
			btm.state.update(&btm.state);

			args[1] = &btm.state.max_free_order;
			update_args();
			printf("%s: allocating a block of order %d...\n", args);

			if (!btm.state.max_free_order) goto err_no_merging;

			btm.block = buddy_alloc(btm.state.max_free_order);

			btm.state.update(&btm.state);

			btm.shadow_state
				.free_lists_lens[btm.state.max_free_order]--;
			btm.shadow_block.order = btm.state.max_free_order;
			btm.shadow_block.refcnt = 1;
			btm.shadow_block.next = NULL;

			break;

		case free_max_order:
			update_args();
			printf("%s: and freeing it...\n", args);

			buddy_free(btm.block);

			btm.state.update(&btm.state);

			btm.shadow_state
				.free_lists_lens[btm.state.max_free_order]++;

			btm.shadow_block.refcnt = 0;
			/* This is effectively a "don't care" */
			btm.shadow_block.next = btm.block->next;
			break;

		case alloc_order_zero:
			/* Make sure there is no page frame available */
			while (btm.state.free_lists_lens[0]) {
				if (!buddy_alloc(0))
					goto err_alloc_available_frames;
				btm.state.update(&btm.state);
			}

			btm.shadow_state.free_lists_lens[0] = 0;
			tmp = btm.state; /* Saving for the next test */

			/* Emulating a split */
			for (size_t i = 0;
			     i < ARRAY_SIZE(btm.state.free_lists_lens); i++) {
				if (btm.state.free_lists_lens[i]) {
					btm.shadow_state.free_lists_lens[i]--;
					break;
				} else {
					btm.shadow_state.free_lists_lens[i]++;
				}
			}

			update_args();
			printf("%s: allocating a block of order 0x0...\n",
			       args);

			btm.block = buddy_alloc(0);
			btm.state.update(&btm.state);

			btm.shadow_block.order = 0;
			btm.shadow_block.refcnt = 1;
			btm.shadow_block.next = NULL;

			break;

		case free_order_zero:
			update_args();
			printf("%s: and freeing it...\n", args);

			buddy_free(btm.block);
			btm.state.update(&btm.state);

			btm.shadow_state = tmp;

			/*
			 * We cannot expect btm.block to be free, as instead
			 * it's buddy might be free now, depends on which block
			 * is pushed on the free list first. That's why we're
			 * not really checking the block here (pointed out by
			 * Sandro)
			 */
			btm.shadow_block = *btm.block;

			break;

		case random_alloc_free:
			update_args();
			int tmp = BUDDY_TEST_RANDOM_NUMBER_OF_TESTS;
			args[1] = &tmp;
			printf("%s: performing %d random sequences of allocs and frees...\n",
			       args);

			for (size_t i = 0;
			     i < BUDDY_TEST_RANDOM_NUMBER_OF_TESTS; i++) {
				buddy_test_machine_random_init(&btm.btmr,
							       btm.state, i);

				ret = buddy_test_machine_random_do(&btm);

				switch (ret) {
				case -1:
					goto missing_block_failure;
				case -2:
					goto block_failure;
				case -3:
					goto state_failure;
				}
			}

			buddy_reset();
			/*
			 * Buddy reset() affects block, but not shadow_block
			 * because shadow_block is on the stack, tricky
			 */
			btm.shadow_block = *btm.block;

			break;

		default:
			goto should_not_happen;
		}

		if (!buddy_block_is_equal(btm.block, &btm.shadow_block))
			goto block_failure;

		if (!buddy_state_is_equal(&btm.state, &btm.shadow_state))
			goto state_failure;

		test_score_update(&tm, ts, tm.t);
		test_print_success(ts[tm.t].name, tm.scores[tm.t],
				   tm.total_score);
	}

	buddy_reset();
	return 0;

should_not_happen:
	return -1;
err_no_merging:
	printstr("\terror: hmmm did you implement merging?\n");
	return -1;
err_alloc_available_frames:
	printstr(
		"\terror: could not allocate page frames from free list (i.e., no splitting)\n");
	return -1;
missing_block_failure:
	printstr("\treason: buddy_alloc() returned NULL?\n");
	buddy_block_print_debug(btm.block, &btm.shadow_block);
	return -1;
block_failure:
	test_print_failure(ts[tm.t].name, tm.total_score);
	printstr("\treason: invalid block\n");
	buddy_block_print_debug(btm.block, &btm.shadow_block);
	return -1;
state_failure:
	test_print_failure(ts[tm.t].name, tm.total_score);
	printstr("\treason: invalid state\n");
	buddy_state_print_debug(&btm.state, &btm.shadow_state);
	return -1;
}
