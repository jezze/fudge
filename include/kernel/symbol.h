#ifndef KERNEL_SYMBOL_H
#define KERNEL_SYMBOL_H

struct kernel_symbol
{

    char name[32];
    void *paddress;

};

extern void *symbol_find(char *name);
extern void symbol_init();

#endif

