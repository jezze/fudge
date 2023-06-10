#include <fudge.h>
#include "gdt.h"

static struct gdt_descriptor *getdescriptor(struct gdt_pointer *pointer, unsigned char index)
{

    return (struct gdt_descriptor *)(pointer->base0 | pointer->base1 << 8 | pointer->base2 << 16 | pointer->base3 << 24) + index;

}

unsigned short gdt_getselector(struct gdt_pointer *pointer, unsigned char index)
{

    struct gdt_descriptor *descriptor = getdescriptor(pointer, index);

    return (sizeof (struct gdt_descriptor) * index) | ((descriptor->access >> 5) & 0x03);

}

void gdt_setdescriptor(struct gdt_pointer *pointer, unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
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

}

void gdt_init(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors)
{

    unsigned int base = (unsigned int)descriptors;
    unsigned short limit = (sizeof (struct gdt_descriptor) * count) - 1;

    pointer->base0 = base;
    pointer->base1 = base >> 8;
    pointer->base2 = base >> 16;
    pointer->base3 = base >> 24;
    pointer->limit0 = limit;
    pointer->limit1 = limit >> 8;

    buffer_clear(descriptors, sizeof (struct gdt_descriptor) * count);

}

