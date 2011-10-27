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
extern struct elf_program_header *elf_get_program_header(void *phaddress);
extern struct elf_section_header *elf_get_section_header_by_index(void *shaddress, unsigned int shsize, unsigned int index);
extern struct elf_section_header *elf_get_section_header_by_type(void *shaddress, unsigned int shsize, unsigned int shcount, unsigned int type);
extern char *elf_get_string_table(void *address, void *shaddress, unsigned int shsize, unsigned int shstringindex);
extern void elf_print_sections(void *shaddress, unsigned int shsize, unsigned int shcount, char *strtbl);
extern void elf_print_symtab(void *address, struct elf_section_header *header);
extern void elf_relocate(void *address, struct elf_header *header);
extern void *elf_get_entry(void *address);
extern void *elf_get_virtual(void *address);
extern void elf_init();

#endif

