#include <fudge/kernel.h>
#include "gdt.h"

#define GDT_ENTRY_SLOTS                 6

static struct gdt_entry entries[GDT_ENTRY_SLOTS];
static struct gdt_pointer pointer;

unsigned short gdt_set_entry(struct gdt_pointer *p, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    p->base[index].baseLow = (base & 0xFFFF);
    p->base[index].baseMiddle = (base >> 16);
    p->base[index].baseHigh = (base >> 24);
    p->base[index].limitLow = (limit & 0xFFFF);
    p->base[index].limitHigh = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    p->base[index].access = access;

    return (sizeof (struct gdt_entry) * index) | ((access >> 5) & 0x03);

}

struct gdt_pointer *gdt_setup_pointer()
{

    pointer.base = entries;
    pointer.limit = (sizeof (struct gdt_entry) * GDT_ENTRY_SLOTS) - 1;

    return &pointer;

}

