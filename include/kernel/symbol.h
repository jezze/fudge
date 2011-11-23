#ifndef KERNEL_SYMBOL_H
#define KERNEL_SYMBOL_H

#define SYMBOL_BUFFER_SIZE 1024

struct symbol_entry
{

    char name[32];
    void *paddress;

};

extern void *symbol_find(char *name);
extern void symbol_init();

#endif

