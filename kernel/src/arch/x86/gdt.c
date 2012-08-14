#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>

static struct gdt_entry entries[GDT_TABLE_SLOTS];
static struct gdt_pointer pointer;

unsigned int gdt_get_segment(unsigned int index)
{

    return (sizeof (struct gdt_entry) * index) | ((entries[index].access >> 5) & 0x03);

}

void gdt_set_gate(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags)
{

    entries[index].baseLow = (base & 0xFFFF);
    entries[index].baseMiddle = (base >> 16);
    entries[index].baseHigh = (base >> 24);
    entries[index].limitLow = (limit & 0xFFFF);
    entries[index].limitHigh = (limit >> 16) | flags;
    entries[index].access = access;

}

void gdt_setup()
{

    memory_clear(&entries, sizeof (struct gdt_entry) * GDT_TABLE_SLOTS);

    gdt_set_gate(GDT_INDEX_NULL, 0x00000000, 0x00000000, 0x00, 0x00);
    gdt_set_gate(GDT_INDEX_KERNELCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_gate(GDT_INDEX_KERNELDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_gate(GDT_INDEX_USERCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    gdt_set_gate(GDT_INDEX_USERDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    pointer.base = entries;
    pointer.limit = (sizeof (struct gdt_entry) * GDT_TABLE_SLOTS) - 1;

    cpu_set_gdt(&pointer);

}

