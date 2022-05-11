#include "util.h"
#include "buddy.h"
#include "pt.h"
#include "test.h"

void main()
{
	int ret;

	buddy_init();

#ifdef TEST
	buddy_test();
#endif

	ret = pt_init();
	if (ret) printdbg("pt init failed: ", (void *)(uintptr_t)ret);
	/*
	 * Why do you think that we call this outside of pt_init and not
	 * inside?
	 */
	pt_jump_to_high();
	pt_destroy_low_kvmas();

#ifdef TEST
	pt_test();
#endif

	printstr("done!\n");

	/* FIXME: should shutdown */
	for (;;) {
	}
	/* Trick to make it stop the infinite main-loop */
};