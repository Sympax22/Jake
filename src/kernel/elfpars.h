#ifndef ELF_PARS
#define ELF_PARS

#include "types.h"
#include "util.h"

extern uintptr_t _testing, _hello_jr;

#define RISCV_ELF 243 
#define ET_REL      1
#define ET_EXEC     2

struct elf_header 
{
	unsigned char e_ident[16]; 
	__u16 e_type;
	__u16 e_machine;
	__u32 e_version;
	__u64 e_entry; 
	__u64 e_phoff; 
	__u64 e_shoff; 
	__u32 e_flags;
	__u16 e_ehsize;
	__u16 e_phentsize;
	__u16 e_phnum;
	__u16 e_shentsize;
	__u16 e_shnum;
	__u16 e_shstrndx;
};

#define ELFMAG "\177ELF"
#define SELFMAG 4

struct elf_simple 
{
	uintptr_t e_entry;      // virtual
	uintptr_t size_load;    // size in the file
	uintptr_t offset_load;  // offset in the file
	uintptr_t virtual_load; // virtual
	int       valid;        // indicates if the parsed elf is valid (FIXME: currently not used)
};

struct elf_jake 
{
	struct elf_simple elf;
	char *ptr_elf;
};

#define PT_LOAD 0x00000001U

int parse_header(struct elf_header *head, void *ptr, unsigned int len);
int parse_elf(char *ptr_elf, struct elf_jake *elf);

#endif