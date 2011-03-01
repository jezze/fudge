#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

struct elf_header;

extern unsigned int elf_check(void *address);
extern void elf_execute(struct elf_header *header, int argc, char *argv[]);
extern void elf_init();

#endif

