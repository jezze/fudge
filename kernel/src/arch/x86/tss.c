#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>

static struct tss_entry entry;

void tss_setup(unsigned int stack)
{

    memory_clear(&entry, sizeof (struct tss_entry));

    entry.ss0 = gdt_get_segment(GDT_INDEX_KERNELDATA);
    entry.esp0 = stack;

    gdt_set_gate(GDT_INDEX_TSS, (unsigned int)&entry, (unsigned int)&entry + sizeof (struct tss_entry), GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_EXECUTE | GDT_ACCESS_USED, 0x00);

    cpu_enable_tss();

}

