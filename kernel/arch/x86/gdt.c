#include <lib/memory.h>
#include <kernel/arch/x86/gdt.h>

struct gdt_entry gdt[GDT_TABLE_SIZE];
struct gdt_ptr gdtPointer;
struct tss_entry tss;

void gdt_set_gate(unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity)
{

    gdt[index].baseLow = (base & 0xFFFF);
    gdt[index].baseMiddle = (base >> 16) & 0xFF;
    gdt[index].baseHigh = (base >> 24) & 0xFF;
    gdt[index].limitLow = (limit & 0xFFFF);
    gdt[index].granularity = (limit >> 16) & 0x0F;
    gdt[index].granularity |= (granularity & 0xF0);
    gdt[index].access = access;

}

void set_kernel_stack(unsigned int stack)
{

    tss.esp0 = stack;

}

void gdt_init()
{

    gdtPointer.base = (unsigned int)&gdt;
    gdtPointer.limit = (sizeof (struct gdt_entry) * GDT_TABLE_SIZE) - 1;

    struct gdt_ptr tssPointer;
    tssPointer.base = (unsigned int)&tss;
    tssPointer.limit = tssPointer.base + sizeof (struct tss_entry);

    memory_set(&gdt, 0, sizeof (struct gdt_entry) * GDT_TABLE_SIZE);
    memory_set(&tss, 0, sizeof (struct tss_entry));

    tss.ss0 = 0x10;
    tss.esp0 = 0x0;
    tss.cs = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    gdt_set_gate(0x00, 0x00000000, 0x00000000, 0x00, 0x00); // Null segment
    gdt_set_gate(0x01, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    gdt_set_gate(0x02, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    gdt_set_gate(0x03, 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    gdt_set_gate(0x04, 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment
    gdt_set_gate(0x05, tssPointer.base, tssPointer.limit, 0xE9, 0x00); // TSS segment

    gdt_flush(&gdtPointer);
    tss_flush();

}

