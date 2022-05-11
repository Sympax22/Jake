#pragma once

typedef unsigned long size_t;
typedef unsigned long uintptr_t;
typedef long intptr_t;
typedef long ssize_t;

/* Macros inspired by the Linux kernel */
#define NULL 0
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define offsetof(struct_name, member_name)                                     \
	((size_t)((char *)&(((struct_name *)0)->member_name)))
#define member_to_struct(member_ptr, member_name, struct_name)                 \
	((struct_name *)((char *)(member_ptr) -                                \
			 (char *)offsetof(struct_name, member_name)))

struct node {
	struct node *next;
	struct node *prev;
};
