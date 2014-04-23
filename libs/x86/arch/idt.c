#include <kernel.h>
#include "idt.h"

void idt_set_descriptor(struct idt_pointer *pointer, enum idt_index index, void (*callback)(), unsigned short selector, enum idt_flag flags)
{

    unsigned long base = (unsigned long)callback;

    pointer->descriptors[index].base0 = base;
    pointer->descriptors[index].base1 = base >> 8;
    pointer->descriptors[index].base2 = base >> 16;
    pointer->descriptors[index].base3 = base >> 24;
    pointer->descriptors[index].selector0 = selector;
    pointer->descriptors[index].selector1 = selector >> 8;
    pointer->descriptors[index].flags = flags;

}

void idt_init_pointer(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors)
{

    memory_clear(pointer, sizeof (struct idt_pointer));

    pointer->descriptors = descriptors;
    pointer->limit = (sizeof (struct idt_descriptor) * count) - 1;

}

