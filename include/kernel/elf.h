#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

unsigned int elf_get_entry(void *address);
unsigned int elf_get_virtual(void *address);
unsigned int elf_get_symbol(void *address, char *name);
void elf_prepare(void *address);
void elf_relocate(void *address);

#endif

