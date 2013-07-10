#include <fudge/kernel.h>
#include "idt.h"

void idt_set_entry(struct idt_pointer *pointer, enum idt_index index, void (*callback)(), unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    pointer->base[index].base0 = base;
    pointer->base[index].base1 = base >> 16;
    pointer->base[index].selector = selector;
    pointer->base[index].flags = flags;

}

void idt_init_pointer(struct idt_pointer *pointer, unsigned int count, struct idt_entry *entries)
{

    pointer->base = entries;
    pointer->limit = (sizeof (struct idt_entry) * count) - 1;

}

