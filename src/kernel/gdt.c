#include <lib/types.h>
#include <kernel/mem.h>
#include <kernel/gdt.h>

extern void gdt_flush();
extern void tss_flush();

gdt_entry_t gdt[6];
gdt_ptr_t gdtp;

tss_entry_t tss;

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{

    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;    

}

void set_kernel_stack(uint32_t stack)
{

    tss.esp0 = stack;

}

void gdt_init()
{

    gdtp.base = (uint32_t)&gdt;
    gdtp.limit = (sizeof (gdt_entry_t) * 6) - 1;

    memset(&gdt, 0, sizeof (gdt_entry_t) * 6);
    memset(&tss, 0, sizeof (tss_entry_t));

    tss.ss0 = 0x10;
    tss.esp0 = 0x0;
    tss.cs = 0x0b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    gdt_set_gate(0, 0, 0, 0, 0);            // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    // Kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    // Kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    // User code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    // User data segment

    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof (tss_entry_t);

    gdt_set_gate(5, base, limit, 0xE9, 0x00);    // TSS

    gdt_flush();
    tss_flush();

}

