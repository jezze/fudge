#include <lib/memory.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/tss.h>

static struct tss_entry tssEntry;

void tss_set_stack(void *address)
{

    tssEntry.esp0 = (unsigned int)address;

}

void tss_init()
{

    memory_set(&tssEntry, 0, sizeof (struct tss_entry));

    tssEntry.ss0 = 0x10;
    tssEntry.esp0 = 0x00;
    tssEntry.cs = 0x0B;
    tssEntry.ss = tssEntry.ds = tssEntry.es = tssEntry.fs = tssEntry.gs = 0x13;

    unsigned int base = (unsigned int)&tssEntry;
    unsigned int limit = base + sizeof (struct tss_entry);

    gdt_set_gate(0x05, base, limit, 0xE9, 0x00);

    tss_flush();

}

