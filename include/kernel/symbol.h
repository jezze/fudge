#ifndef KERNEL_SYMBOL_H
#define KERNEL_SYMBOL_H

#define SYMBOL_ENTRY_SLOTS 64
#define SYMBOL_BUFFER_SIZE 1024

struct symbol_entry
{

    char name[32];
    void *paddress;

};

void *symbol_find(char *name);
void symbol_init();

#endif

