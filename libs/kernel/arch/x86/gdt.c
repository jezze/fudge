#include <fudge/kernel.h>
#include "gdt.h"

static struct gdt_entry entries[GDT_ENTRY_SLOTS];
static struct gdt_pointer pointer;

unsigned short gdt_set_entry(struct gdt_entry *entry, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    entry->baseLow = (base & 0xFFFF);
    entry->baseMiddle = (base >> 16);
    entry->baseHigh = (base >> 24);
    entry->limitLow = (limit & 0xFFFF);
    entry->limitHigh = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    entry->access = access;

    return (sizeof (struct gdt_entry) * index) | ((entry->access >> 5) & 0x03);

}

struct gdt_pointer *gdt_setup_pointer()
{

    memory_clear(&entries, sizeof (struct gdt_entry) * GDT_ENTRY_SLOTS);

    pointer.base = entries;
    pointer.limit = (sizeof (struct gdt_entry) * GDT_ENTRY_SLOTS) - 1;

    return &pointer;

}

