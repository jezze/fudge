#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/gdt.h>

gdt_entry_t gdt[GDT_TABLE_SIZE];
gdt_ptr_t gdtp;
tss_entry_t tss;

void gdt_set_gate(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{

    gdt[index].baseLow = (base & 0xFFFF);
    gdt[index].baseMiddle = (base >> 16) & 0xFF;
    gdt[index].baseHigh = (base >> 24) & 0xFF;
    gdt[index].limitLow = (limit & 0xFFFF);
    gdt[index].granularity = (limit >> 16) & 0x0F;
    gdt[index].granularity |= (gran & 0xF0);
    gdt[index].access = access;    

}

void set_kernel_stack(uint32_t stack)
{

    tss.esp0 = stack;

}

void gdt_init()
{

    gdtp.base = (uint32_t)&gdt;
    gdtp.limit = (sizeof (gdt_entry_t) * GDT_TABLE_SIZE) - 1;

    gdt_ptr_t tssp;
    tssp.base = (uint32_t)&tss;
    tssp.limit = tssp.base + sizeof (tss_entry_t);

    memory_set(&gdt, 0, sizeof (gdt_entry_t) * GDT_TABLE_SIZE);
    memory_set(&tss, 0, sizeof (tss_entry_t));

    tss.ss0 = 0x10;
    tss.esp0 = 0x0;
    tss.cs = 0x0b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    gdt_set_gate(0, 0, 0x00000000, 0x00, 0x00); // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment
    gdt_set_gate(5, tssp.base, tssp.limit, 0xE9, 0x00); // TSS segment

    gdt_flush();
    tss_flush();

}

