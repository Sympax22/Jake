#ifndef UTIL_H
#define UTIL_H

#include "types.h"

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

/* clang-format off */
#define YAK_LOGO                                                                   \
RED"    __ _____ _____ _____ \n"\
GRN" __|  |  _  |  |  |   __|\n"\
YEL"|  |  |     |    -|   __|\n"\
BLU"|_____|__|__|__|__|_____|\n\n"RESET
/* clang-format on */

#define ra_read(out) asm volatile("mv %0, ra" : "=r"(out)::"memory")
#define sp_read(out) asm volatile("mv %0, sp" : "=r"(out)::"memory")
#define csrw(name, in) asm volatile("csrw " #name ", %0" ::"r"(in) : "memory")
#define csrr(name, out) asm volatile("csrr %0, " #name : "=r"(out)::"memory")
#define read_r(name, out) asm volatile("mv %0, " #name : "=r"(out)::"memory")
#define write_r(in, name) asm volatile("mv " #name ", %0" ::"r"(in) : "memory")
#define list_get_struct(head_ptr, i, struct_name)                              \
	(member_to_struct(list_get(head_ptr, i), node, struct_name))

#define PLOC 1
#define ploc() ploc2("")
#define ploc2(tag)                                                             \
	do {                                                                   \
		if (PLOC) {                                                    \
			printstr(__FILE__);                                    \
			printstr(":");                                         \
			printstr(__func__);                                    \
			printstr(":");                                         \
			printstr(tag);                                         \
			printstr("\n");                                        \
		}                                                              \
	} while (0)

/* Arrays */
void memcpy(void *dest, const void *src, size_t n);
void memset(void *s, int c, size_t n);
void reverse(char *buf, size_t len);

/* Strings */
size_t strlen(const char *str);
int strcmp(char *str1, char *str2);
int strncmp(char *str1, char *str2, unsigned int n);

/* Printing */
#define printstr(...) __printstr(__VA_ARGS__)
#define printerr(...) printstr(RED __VA_ARGS__ RESET)

void put(const char);

void printdbg(const char *str, const void *ptr);
void printf(const char *fmt, const void **args);
void printptr(const void *ptr, char end);

void __printstr(const char *str);
void __printnum(uintptr_t ptr);
/* Random numbers */
unsigned rand(unsigned seed);

/* Bit operations */
int ctz(uintptr_t in);

/* Linked list */
void node_init(struct node *node);
int is_node_init(struct node *node);
void node_migrate(struct node *node, uintptr_t offset);
void list_append(struct node *head, struct node *node);
void list_remove(struct node *node);
int list_is_empty(struct node *head);

#endif