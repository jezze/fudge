#include <lib/memory.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/tss.h>

static struct tss_entry tss;

void tss_set_stack(void *address)
{

    tss.esp0 = (unsigned int)address;

}

void tss_init()
{

    memory_set(&tss, 0, sizeof (struct tss_entry));

    tss.ss0 = 0x10;
    tss.esp0 = 0x00;
    tss.cs = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    unsigned int base = (unsigned int)&tss;
    unsigned int limit = base + sizeof (struct tss_entry);

    gdt_set_gate(0x05, base, limit, 0xE9, 0x00);

    tss_flush();

}

