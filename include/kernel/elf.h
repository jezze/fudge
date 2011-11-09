#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

extern struct elf_header *elf_get_header(void *address);
extern void *elf_get_entry(void *address);
extern void *elf_get_virtual(void *address);
extern void elf_relocate(void *address);
extern void elf_init();

#endif

