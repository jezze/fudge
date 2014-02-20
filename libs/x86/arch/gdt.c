#include <kernel.h>
#include "gdt.h"

unsigned short gdt_set_descriptor(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    pointer->descriptors[index].base0 = base;
    pointer->descriptors[index].base1 = base >> 16;
    pointer->descriptors[index].base2 = base >> 24;
    pointer->descriptors[index].limit0 = limit;
    pointer->descriptors[index].limit1 = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    pointer->descriptors[index].access = access;

    return (sizeof (struct gdt_descriptor) * index) | ((access >> 5) & 0x03);

}

void gdt_init_pointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors)
{

    memory_clear(pointer, sizeof (struct gdt_pointer));

    pointer->descriptors = descriptors;
    pointer->limit = (sizeof (struct gdt_descriptor) * count) - 1;

}

