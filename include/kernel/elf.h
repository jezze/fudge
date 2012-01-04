#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

extern void *elf_get_entry(void *address);
extern void *elf_get_virtual(void *address);
extern void *elf_get_symbol(void *address, char *name);
extern void elf_relocate(void *address);

#endif

