#include <lib/memory.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/gdt.h>

static struct gdt_entry gdtEntries[GDT_TABLE_SLOTS];
static struct gdt_pointer gdtPointer;

void gdt_set_gate(unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity)
{

    gdtEntries[index].baseLow = (base & 0xFFFF);
    gdtEntries[index].baseMiddle = (base >> 16) & 0xFF;
    gdtEntries[index].baseHigh = (base >> 24) & 0xFF;
    gdtEntries[index].limitLow = (limit & 0xFFFF);
    gdtEntries[index].granularity = (limit >> 16) & 0x0F;
    gdtEntries[index].granularity |= (granularity & 0xF0);
    gdtEntries[index].access = access;

}

void gdt_init()
{

    memory_set(&gdtEntries, 0, sizeof (struct gdt_entry) * GDT_TABLE_SLOTS);

    gdt_set_gate(0x00, 0x00000000, 0x00000000, 0x00, 0x00); // Null segment
    gdt_set_gate(0x01, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    gdt_set_gate(0x02, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    gdt_set_gate(0x03, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    gdt_set_gate(0x04, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment

    gdtPointer.base = gdtEntries;
    gdtPointer.limit = (sizeof (struct gdt_entry) * GDT_TABLE_SLOTS) - 1;

    cpu_set_gdt(&gdtPointer);

}

