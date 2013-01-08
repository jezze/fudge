#include <fudge/kernel.h>
#include "cpu.h"
#include "gdt.h"

static struct gdt_entry entries[GDT_ENTRY_SLOTS];
static struct gdt_pointer pointer;

unsigned short gdt_get_selector(unsigned int index)
{

    if (index >= GDT_ENTRY_SLOTS)
        return 0;

    return (sizeof (struct gdt_entry) * index) | ((entries[index].access >> 5) & 0x03);

}

void gdt_set_entry(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    if (index >= GDT_ENTRY_SLOTS)
        return;

    entries[index].baseLow = (base & 0xFFFF);
    entries[index].baseMiddle = (base >> 16);
    entries[index].baseHigh = (base >> 24);
    entries[index].limitLow = (limit & 0xFFFF);
    entries[index].limitHigh = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    entries[index].access = access;

}

void gdt_setup()
{

    memory_clear(&entries, sizeof (struct gdt_entry) * GDT_ENTRY_SLOTS);

    gdt_set_entry(GDT_INDEX_NULL, 0x00000000, 0x00000000, 0x00, 0x00);
    gdt_set_entry(GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_entry(GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_entry(GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_entry(GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    pointer.base = entries;
    pointer.limit = (sizeof (struct gdt_entry) * GDT_ENTRY_SLOTS) - 1;

    cpu_set_gdt(&pointer);

}

