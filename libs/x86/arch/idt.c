#include <fudge/kernel.h>
#include "idt.h"

void idt_set_descriptor(struct idt_pointer *pointer, enum idt_index index, void (*callback)(), unsigned short selector, unsigned char flags)
{

    unsigned long base = (unsigned long)callback;

    pointer->descriptors[index].base0 = base;
    pointer->descriptors[index].base1 = base >> 16;
    pointer->descriptors[index].selector = selector;
    pointer->descriptors[index].flags = flags;

}

void idt_init_pointer(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors)
{

    memory_clear(pointer, sizeof (struct idt_pointer));

    pointer->descriptors = descriptors;
    pointer->limit = (sizeof (struct idt_descriptor) * count) - 1;

}

