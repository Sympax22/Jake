#include "timer.h"

/* timer_setup
 * Setup and starts the timer.
 */
inline void timer_setup()
{
	uint64_t mstatus, mie, sie;
	csrr(mstatus,mstatus);
	csrr(mie, mie);
	csrr(sie, sie);
	csrw(mstatus, mstatus|MIE|SIE);
	csrw(mie, mie|MTIE|STIE);
	csrw(sie, sie|MTIE|STIE);

	increase_timer();
}

/* increase_timer
 * Increments the memory mapped compare register
 * for the timer. 
 */
inline void increase_timer()
{
	volatile uintptr_t *mtime = (void *)MTIME;
	volatile uintptr_t *mtimecmp = (void *)MTIMECMP;
	uintptr_t total = 0;
	total = *mtime + PERIOD_TIMER;
	// Timer safe. See manual page 31.
	*mtimecmp = total & MASK_LOW;
	*mtimecmp = total & MASK_HIGH;
}