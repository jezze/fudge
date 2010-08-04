#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

extern void elf_execute(struct elf_header *header, int argc, char *argv[]);
extern unsigned int elf_check(struct elf_header *header);

#endif

