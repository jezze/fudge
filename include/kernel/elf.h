#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

void *elf_get_entry(void *address);
void *elf_get_virtual(void *address);
void *elf_get_symbol(void *address, char *name);
void elf_prepare(void *address);
void elf_relocate(void *address);

#endif

