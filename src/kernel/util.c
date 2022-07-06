#include "util.h"
#include "exception.h"

void put(const char);

/* Char to hex */
void __ctox(const char in, char *left, char *right)
{
	static const char hex[] = "0123456789abcdef";
	*left = hex[(in & 0xf0) >> 4];
	*right = hex[in & 0x0f];
}

/* Chars to hex */
/* NOTE: *out must be twice the size of *in */
static size_t __cstox(const char *in, size_t len, char *out)
{
	size_t firstnonzero = 2 * (len - 1);

	size_t j = 0;
	for (size_t i = 0; i < len; i++) {
		__ctox(in[i], &out[j], &out[j + 1]);
		j += 2;

		firstnonzero =
			2 * i < firstnonzero && in[i] ? 2 * i : firstnonzero;
	}

	return firstnonzero;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len] != '\0') {
		len++;
	}
	return len;
}

void memcpy(void *dest, const void *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		*((unsigned char *)dest + i) = *((unsigned char *)src + i);
	}
}

/*
 * Depends on:
 *         1. Data on page
 *         2. Location of this data
 *         3. The above for all previous calls
 *
 * r += (seed_page[i] * (uintptr_t)seed_page) ^ seed_page[r % magni];
 */
unsigned rand(unsigned seed)
{
	unsigned carol = ((1U << 15) * (1U << 15) - 1) - 2;

	for (unsigned i = 0; i < 8 * sizeof(unsigned); i++) {
		seed = ((seed + 1) * seed) % carol;
	}

	return seed;
}

void memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		*((unsigned char *)s + i) = (unsigned char)c;
	}
}

void reverse(char *buf, size_t len)
{
	for (size_t i = 0; i < len / 2; i++) {
		char tmp = buf[i];
		buf[i] = buf[len - 1 - i];
		buf[len - 1 - i] = tmp;
	}
}

void __printstr(const char *str)
{
	for (size_t i = 0; i < strlen(str); i++) {
		put(str[i]);
	}
}

void __printnum(uintptr_t ptr)
{
	/* Little-endian */
	reverse((char *)&ptr, sizeof(ptr));
	char buf[2 * sizeof(ptr) + 1];
	buf[ARRAY_SIZE(buf) - 1] = '\0';

	size_t firstnonzero = __cstox((char *)&ptr, sizeof(ptr), buf);

	__printstr("0x");
	__printstr(buf + firstnonzero);
}

void printf(const char *fmt, const void **args)
{
	for (size_t i = 0; i < strlen(fmt); i++) {
		switch (fmt[i]) {
		case '\0':
			return;
		case '%':
			switch (fmt[i++ + 1]) {
			case '\0': /* error */
			case 's':
				__printstr((char *)*args);
				break;
			case 'p':
				__printnum((uintptr_t)*args);
				break;
			case 'd':
				/* FIXME: problem with int/uintptr */
				__printnum(*(int *)*args);
				break;
			case 'l': /* FIXME: make ld? */
				__printnum(*(intptr_t *)*args);
				break;
			default:
				put(fmt[i]);
			}

			args++;
			break;
		default:
			put(fmt[i]);
		}
	}
}

void printptr(const void *ptr, char end)
{
	__printnum((uintptr_t)ptr);
	put(end);
}

void printdbg(const char *str, const void *ptr)
{
	printstr(str);
	printstr(MAG);
	printptr(ptr, '\n');
	printstr(RESET);
}

int ctz(uintptr_t in)
{
	int out = 0;
	while (in > 1) {
		in = in >> 1;
		out++;
	}
	return out;
}

/* Inspired by Stephan van Schaik's implementation */
void node_init(struct node *node)
{
	node->next = node;
	node->prev = node;
}

int is_node_init(struct node *node)
{
	if (node->prev && node->next) {
		return 1;
	} else {
		return 0;
	}
}

void node_migrate(struct node *node, uintptr_t offset)
{
	node->next = (struct node *)((char *)node->next + offset);
	node->prev = (struct node *)((char *)node->prev + offset);
}

void list_append(struct node *head, struct node *node)
{
	node->next = head;
	node->prev = head->prev;
	head->prev->next = node;
	head->prev = node;
}

void list_remove(struct node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = node;
	node->prev = node;
}

int list_is_empty(struct node *head)
{
	return head->next == head;
}
/*
	This is strmcp implementation is different from C ANSI where 
	0 is returned if they are equal.
*/
int strcmp(char *str1, char *str2)
{
	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 != *str2) return 0;

		str1++;
		str2++;
	}

	if (*str1 != *str2)
		return 0;
	else
		return 1;
}

// Compares up to n characters. For non-0 ending strings
int strncmp(char *str1, char *str2, unsigned int n)
{
	while (n > 0) {
		if (*str1 != *str2) return 0;

		str1++;
		str2++;
		n--;
	}

	return 1;
}