#include <fudge.h>
#include "idt.h"

void idt_setdescriptor(struct idt_pointer *pointer, unsigned char index, void (*callback)(void), unsigned short selector, unsigned char flags)
{

    struct idt_descriptor *descriptor = (struct idt_descriptor *)(pointer->base0 | pointer->base1 << 8 | pointer->base2 << 16 | pointer->base3 << 24) + index;
    unsigned int base = (unsigned int)callback;

    descriptor->base0 = base;
    descriptor->base1 = base >> 8;
    descriptor->base2 = base >> 16;
    descriptor->base3 = base >> 24;
    descriptor->selector0 = selector;
    descriptor->selector1 = selector >> 8;
    descriptor->flags = flags;

}

void idt_cleardescriptors(struct idt_pointer *pointer, unsigned int count)
{

    struct idt_descriptor *descriptors = (struct idt_descriptor *)(pointer->base0 | pointer->base1 << 8 | pointer->base2 << 16 | pointer->base3 << 24);

    buffer_clear(descriptors, sizeof (struct idt_descriptor) * count);

}

void idt_initpointer(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors)
{

    unsigned int base = (unsigned int)descriptors;
    unsigned short limit = (sizeof (struct idt_descriptor) * count) - 1;

    pointer->base0 = base;
    pointer->base1 = base >> 8;
    pointer->base2 = base >> 16;
    pointer->base3 = base >> 24;
    pointer->limit0 = limit;
    pointer->limit1 = limit >> 8;

}

