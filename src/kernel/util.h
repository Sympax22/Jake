#include "types.h"

#define csrw(name, in) asm volatile("csrw " #name ", %0" ::"r"(in))
#define csrr(name, out) asm volatile("csrr %0, " #name : "=r"(out))
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

/* Printing */
#define printstr(...) __printstr(__VA_ARGS__)

void printdbg(const char *str, const void *ptr);
void printf(const char *fmt, const void **args);
void printptr(const void *ptr, char end);

void __printstr(const char *str);
void __printnum(uintptr_t ptr);

/* Random numbers */
unsigned rand(unsigned seed);

/* Bit operations */
int ctz(uintptr_t in);

/* asm */
uintptr_t sp_read(void);

/* Linked list */
void node_init(struct node *node);
int is_node_init(struct node *node);
void node_migrate(struct node *node, uintptr_t offset);
void list_append(struct node *head, struct node *node);
void list_remove(struct node *node);
int list_is_empty(struct node *head);
