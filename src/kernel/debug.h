#ifndef DEBUG_H
#define DEBUG_H
// Note: in print_call_info we are in S-mode, we cannot access
// M-mode registers (would cause an exception).
void print_call_info(void);
void mprint_call_info(void);
void print_all_registers(void);
#endif