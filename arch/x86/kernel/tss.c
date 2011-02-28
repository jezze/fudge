#include <lib/memory.h>
#include <arch/x86/kernel/gdt.h>
#include <arch/x86/kernel/tss.h>

struct tss_entry tss;

void tss_init()
{

    memory_set(&tss, 0, sizeof (struct tss_entry));

    tss.ss0 = 0x10;
    tss.esp0 = 0x00;
    tss.cs = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;

    unsigned int tssBase = (unsigned int)&tss;
    unsigned int tssLimit = tssBase + sizeof (struct tss_entry);

    gdt_set_gate(0x05, tssBase, tssLimit, 0xE9, 0x00);

    tss_flush();

}

