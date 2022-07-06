#ifndef TYPES_H
#define TYPES_H

typedef unsigned long size_t;
typedef unsigned long uintptr_t;
typedef long intptr_t;
typedef long ssize_t;

typedef __signed__ char __s8;
typedef __signed__ short __s16;
typedef unsigned short __u16;
typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;

typedef unsigned long long uint64_t;

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

#endif