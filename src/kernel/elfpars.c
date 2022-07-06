#include "elfpars.h"

/* parse_header
 * This function can be used to double check things if required. 
 * Curently it is not used, here only for future reference.
 * @head: 	struct to write to the parsed information
 * @ptr: 	pointer to the application binary
 * @len:	length of the binary
 */
// FIXME: We could make head point to the ptr address.. but might be
// very dangerous for the future. Instead we just copy the parts.
int parse_header(struct elf_header *head, void *ptr, unsigned int len)
{
	if (head == NULL) 
	{
		printstr("[!!] head null.\n");
		return -1;
	}

	if (ptr == NULL) 
	{
		printstr("[!!] ptr null.\n");
		return -1;
	}

	if (len == 0) return -1;

	memcpy(head, ptr, sizeof(struct elf_header) * 1);

	if (!strncmp((char *)head->e_ident, ELFMAG, SELFMAG)) 
	{
		printstr("[!!] ELFMAG not found, aborting.\n");
		return -1;
	}

	if (head->e_type != ET_EXEC && head->e_type != ET_REL) 
	{
		printstr("[!!] e_type not ET_EXEC or ET_REL, aborting.\n");
		return -1;
	}

	if (head->e_machine != RISCV_ELF) 
	{
		printstr("[!!] e_machine not RISCV_ELF, aborting.\n");
		return -1;
	}

	return 0;
}

/* parse_elf
 * @ptr_elf:	pointer to the binary data
 * @elf:		pointer to an output elf_jake variable
 * return:	-1 in case of errors, otherwise 0.
 */
int parse_elf(char *ptr_elf, struct elf_jake *elf)
{
	int valid = 0;

	//PT_LOAD in p_type
	uintptr_t e_phoff;
	unsigned short e_phentsize, e_phnum;

	elf->elf.e_entry = *(uintptr_t *)(ptr_elf + 0x18);
	e_phoff          = *(uintptr_t *)(ptr_elf + 0x20);

	e_phentsize      = *(unsigned short *)(ptr_elf + 0x36);
	e_phnum          = *(unsigned short *)(ptr_elf + 0x38);

	void *ptr_ph = ptr_elf + e_phoff;
	unsigned int p_type;

	for (int t = 0; t < e_phnum; t++) 
	{
		p_type = *(unsigned int *)(ptr_ph);

		// FIXME: consider that we might need to load more than one..
		if (p_type == PT_LOAD) 
		{
			valid = 1;
			break;
		}

		ptr_ph += e_phentsize;
	}

	elf->elf.virtual_load = *(uintptr_t *)(ptr_ph + 0x10);
	elf->elf.offset_load  = *(uintptr_t *)(ptr_ph + 0x08);
	elf->elf.size_load    = *(uintptr_t *)(ptr_ph + 0x20);

	elf->elf.valid = valid;

	elf->ptr_elf   = ptr_elf;

	return 0;
}