#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>

static struct tss_entry entries[TSS_TABLE_SLOTS];

void tss_set_entry(unsigned int index, unsigned int ss, unsigned int esp)
{

    if (index >= TSS_TABLE_SLOTS)
        return;

    entries[index].ss0 = ss;
    entries[index].esp0 = esp;

}

void tss_setup(unsigned int ss, unsigned int esp)
{

    memory_clear(&entries, sizeof (struct tss_entry) * TSS_TABLE_SLOTS);

    tss_set_entry(0, ss, esp);
    gdt_set_entry(GDT_INDEX_TSS, (unsigned int)&entries[0], (unsigned int)&entries[0] + sizeof (struct tss_entry), GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    cpu_set_tss(gdt_get_segment(GDT_INDEX_TSS));

}

