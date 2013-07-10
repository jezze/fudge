#include <fudge/kernel.h>
#include "gdt.h"

unsigned short gdt_set_entry(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    pointer->base[index].base0 = base;
    pointer->base[index].base1 = base >> 16;
    pointer->base[index].base2 = base >> 24;
    pointer->base[index].limit0 = limit;
    pointer->base[index].limit1 = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    pointer->base[index].access = access;

    return (sizeof (struct gdt_entry) * index) | ((access >> 5) & 0x03);

}

void gdt_init_pointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_entry *entries)
{

    pointer->base = entries;
    pointer->limit = (sizeof (struct gdt_entry) * count) - 1;

}

