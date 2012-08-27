#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>

static struct tss_entry entry;

void tss_setup(unsigned int ss, unsigned int esp)
{

    memory_clear(&entry, sizeof (struct tss_entry));

    entry.ss0 = ss;
    entry.esp0 = esp;

    gdt_set_gate(GDT_INDEX_TSS, (unsigned int)&entry, (unsigned int)&entry + sizeof (struct tss_entry), GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    cpu_set_tss(gdt_get_segment(GDT_INDEX_TSS));

}

