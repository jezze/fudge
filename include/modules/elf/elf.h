#ifndef MODULES_ELF_H
#define MODULES_ELF_H

struct elf_header;

struct elf_module
{

    struct modules_module base;
    void *(*get_entry)(void *address);
    void *(*get_virtual)(void *address);

};

extern void *elf_get_entry(void *address);
extern void *elf_get_virtual(void *address);
extern void elf_init();

#endif

