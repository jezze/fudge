#include <fudge.h>
#include "idt.h"

static struct idt_descriptor *getdescriptor(struct idt_pointer *pointer, unsigned short index)
{

    unsigned long base = (unsigned long)pointer->base0 | (unsigned long)pointer->base1 << 8 | (unsigned long)pointer->base2 << 16 | (unsigned long)pointer->base3 << 24 | (unsigned long)pointer->base4 << 32 | (unsigned long)pointer->base5 << 40 | (unsigned long)pointer->base6 << 48 | (unsigned long)pointer->base7 << 56;

    return (struct idt_descriptor *)base + index;

}

void idt_setdescriptor(struct idt_pointer *pointer, unsigned short index, void (*callback)(void), unsigned short selector, unsigned char flags)
{

    struct idt_descriptor *descriptor = getdescriptor(pointer, index);
    unsigned long base = (unsigned long)callback;

    descriptor->base0 = base;
    descriptor->base1 = base >> 8;
    descriptor->base2 = base >> 16;
    descriptor->base3 = base >> 24;
    descriptor->base4 = base >> 32;
    descriptor->base5 = base >> 40;
    descriptor->base6 = base >> 48;
    descriptor->base7 = base >> 56;
    descriptor->selector0 = selector;
    descriptor->selector1 = selector >> 8;
    descriptor->flags = flags;

}

void idt_init(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors)
{

    unsigned long base = (unsigned long)descriptors;
    unsigned short limit = (sizeof (struct idt_descriptor) * count) - 1;

    pointer->base0 = base;
    pointer->base1 = base >> 8;
    pointer->base2 = base >> 16;
    pointer->base3 = base >> 24;
    pointer->base4 = base >> 32;
    pointer->base5 = base >> 40;
    pointer->base6 = base >> 48;
    pointer->base7 = base >> 56;
    pointer->limit0 = limit;
    pointer->limit1 = limit >> 8;

    buffer_clear(descriptors, sizeof (struct idt_descriptor) * count);

}
