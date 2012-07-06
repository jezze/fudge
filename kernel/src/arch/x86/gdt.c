#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>

static struct gdt_entry entries[GDT_TABLE_SLOTS];
static struct gdt_pointer pointer;

void gdt_set_gate(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity)
{

    entries[index].baseLow = (base & 0xFFFF);
    entries[index].baseMiddle = (base >> 16) & 0xFF;
    entries[index].baseHigh = (base >> 24) & 0xFF;
    entries[index].limitLow = (limit & 0xFFFF);
    entries[index].granularity = (limit >> 16) & 0x0F;
    entries[index].granularity |= (granularity & 0xF0);
    entries[index].access = access;

}

void gdt_setup()
{

    memory_clear(&entries, sizeof (struct gdt_entry) * GDT_TABLE_SLOTS);

    gdt_set_gate(GDT_INDEX_NULL, 0x00000000, 0x00000000, 0x00, 0x00);
    gdt_set_gate(GDT_INDEX_KERNELCODE, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(GDT_INDEX_KERNELDATA, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(GDT_INDEX_USERCODE, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(GDT_INDEX_USERDATA, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF);

    pointer.base = entries;
    pointer.limit = (sizeof (struct gdt_entry) * GDT_TABLE_SLOTS) - 1;

    cpu_set_gdt(&pointer);

}

