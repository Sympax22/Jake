#ifndef TIMER_H
#define TIMER_H

#include "types.h"
#include "util.h"

// CLINT timer works@1MHz
#define PERIOD_TIMER 100000

// Memory mapped timer
#define MTIME 0x200BFF8
#define MTIMECMP 0x2004000

// Machine Interrupt Enable
#define MIE (1UL<<3)

// Supervisor Interrupt Enable
#define SIE (1UL<<1)

// Machine Timer Interrupt Enable
#define MTIE (1UL<<7)

// Supervisor Timer Interrupt Enable
#define STIE (1UL<<5)

#define MASK_LOW 0xFFFFFFFUL
#define MASK_HIGH 0xFFFFFFFFFFFFFFUL

void timer_setup();
void increase_timer();

#endif