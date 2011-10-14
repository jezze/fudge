#ifndef MODULES_ELF_H
#define MODULES_ELF_H

struct elf_header;

struct elf_module
{

    struct modules_module base;

};

extern struct elf_header *elf_get_header(void *address);
extern struct elf_program_header *elf_get_program_header(struct elf_header *header);
extern void elf_init();

#endif

