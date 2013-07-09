#include <fudge/kernel.h>
#include "gdt.h"

unsigned short gdt_set_entry(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    pointer->base[index].baseLow = (base & 0xFFFF);
    pointer->base[index].baseMiddle = (base >> 16);
    pointer->base[index].baseHigh = (base >> 24);
    pointer->base[index].limitLow = (limit & 0xFFFF);
    pointer->base[index].limitHigh = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    pointer->base[index].access = access;

    return (sizeof (struct gdt_entry) * index) | ((access >> 5) & 0x03);

}

void gdt_init_pointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_entry *entries)
{

    pointer->base = entries;
    pointer->limit = (sizeof (struct gdt_entry) * count) - 1;

}

