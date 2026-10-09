#include <fudge.h>
#include "gdt.h"

static struct gdt_descriptor *getdescriptor(struct gdt_pointer *pointer, unsigned short index)
{

    unsigned long base = (unsigned long)pointer->base0 | (unsigned long)pointer->base1 << 8 | (unsigned long)pointer->base2 << 16 | (unsigned long)pointer->base3 << 24 | (unsigned long)pointer->base4 << 32 | (unsigned long)pointer->base5 << 40 | (unsigned long)pointer->base6 << 48 | (unsigned long)pointer->base7 << 56;

    return (struct gdt_descriptor *)base + index;

}

unsigned short gdt_getselector(struct gdt_pointer *pointer, unsigned short index)
{

    struct gdt_descriptor *descriptor = getdescriptor(pointer, index);

    return (sizeof (struct gdt_descriptor) * index) | ((descriptor->access >> 5) & 0x03);

}

void gdt_setdescriptor(struct gdt_pointer *pointer, unsigned short index, unsigned long base, unsigned int limit, unsigned char access, unsigned char flags)
{

    struct gdt_descriptor *descriptor = getdescriptor(pointer, index);

    descriptor->base0 = base;
    descriptor->base1 = base >> 8;
    descriptor->base2 = base >> 16;
    descriptor->base3 = base >> 24;
    descriptor->limit0 = limit;
    descriptor->limit1 = limit >> 8;
    descriptor->limit2 = flags | ((limit >> 16) & 0x0F);
    descriptor->access = access;

    if (!(access & GDT_ACCESS_ALWAYS1))
    {

        struct gdt_descriptor *upper = descriptor + 1;

        upper->limit0 = base >> 32;
        upper->limit1 = base >> 40;
        upper->base0 = base >> 48;
        upper->base1 = base >> 56;

    }

}

void gdt_init(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors)
{

    unsigned long base = (unsigned long)descriptors;
    unsigned short limit = (sizeof (struct gdt_descriptor) * count) - 1;

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

    buffer_clear(descriptors, sizeof (struct gdt_descriptor) * count);

}
