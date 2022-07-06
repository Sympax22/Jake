#include "util.h"
#include "buddy.h"
#include "pt.h"
#include "process.h"
#include "test.h"
#include "exception.h"

/* TODO 9: Written by us
 * main
 */
void main()
{
	printstr(YAK_LOGO);
	buddy_init();

#ifdef TEST
	buddy_test();
#endif

	int ret;
	ret = pt_init();
	if (ret != 0) 
	{
		printdbg("pt init failed: ", (void *)(uintptr_t)ret);
		turn_off;
	}

	pt_jump_to_high();
	pt_destroy_low_kvmas();

#ifdef TEST
	pt_test();
#endif

	proc_init_process_list();

	set_sscratch();

	/* TODO 9:
	**  When you are ready, enable the timer interrupt by uncommenting the following line
	**/
	ecall_timer_setup();

	// FIXME: why doesnt it work if it's used
	// directly?
	void *ptr = &_testing;
	printstr("Starting testing process...: \n");
	ret       = proc_run_process(ptr, 0);

	if (ret) 
	{
		printdbg("ERROR: Could not test process: \n",
			 (void *)(uintptr_t)ret);
	}
}
