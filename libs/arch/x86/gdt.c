#include <kernel.h>
#include "gdt.h"

unsigned short gdt_setdescriptor(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, enum gdt_access access, enum gdt_flag flags)
{

    struct gdt_descriptor *descriptor = (struct gdt_descriptor *)(pointer->base0 | pointer->base1 << 8 | pointer->base2 << 16 | pointer->base3 << 24) + index;

    descriptor->base0 = base;
    descriptor->base1 = base >> 8;
    descriptor->base2 = base >> 16;
    descriptor->base3 = base >> 24;
    descriptor->limit0 = limit;
    descriptor->limit1 = limit >> 8;
    descriptor->limit2 = flags | ((limit >> 16) & 0x0F);
    descriptor->access = access;

    return (sizeof (struct gdt_descriptor) * index) | ((access >> 5) & 0x03);

}

void gdt_initpointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors)
{

    unsigned long base = (unsigned long)descriptors;
    unsigned short limit = (sizeof (struct gdt_descriptor) * count) - 1;

    pointer->base0 = base;
    pointer->base1 = base >> 8;
    pointer->base2 = base >> 16;
    pointer->base3 = base >> 24;
    pointer->limit0 = limit;
    pointer->limit1 = limit >> 8;

}

