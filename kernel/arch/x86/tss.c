#include <lib/memory.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/tss.h>

static struct tss_entry entry;

void tss_init(unsigned int stack)
{

    memory_clear(&entry, sizeof (struct tss_entry));

    entry.ss0 = 0x10;
    entry.esp0 = stack;
    entry.cs = 0x0B;
    entry.ss = entry.ds = entry.es = entry.fs = entry.gs = 0x13;

    gdt_set_gate(0x05, (unsigned int)&entry, (unsigned int)&entry + sizeof (struct tss_entry), 0xE9, 0x00);

    cpu_enable_tss();

}

