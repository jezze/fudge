#ifndef MODULES_ELF_H
#define MODULES_ELF_H

struct elf_header;

struct elf_module
{

    struct modules_module base;
    void *(*get_entry)(void *address);
    void *(*get_virtual)(void *address);

};

extern struct elf_header *elf_get_header(void *address);
extern struct elf_program_header *elf_get_program_header(void *address);
extern struct elf_section_header *elf_get_section_header_by_index(void *address, unsigned int index);
extern void *elf_get_entry(void *address);
extern void *elf_get_virtual(void *address);
extern void *elf_get_symbol(void *address, char *symname);
extern void elf_relocate(void *address);
extern void elf_init();

#endif

