#include <memory.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>

static struct tss_entry entries[TSS_ENTRY_SLOTS];

void tss_set_entry(unsigned int index, unsigned int selector, unsigned int stack)
{

    if (index >= TSS_ENTRY_SLOTS)
        return;

    entries[index].ss0 = selector;
    entries[index].esp0 = stack;

}

void tss_setup(unsigned short selector, unsigned int stack)
{

    memory_clear(&entries, sizeof (struct tss_entry) * TSS_ENTRY_SLOTS);

    tss_set_entry(0, selector, stack);
    gdt_set_entry(GDT_INDEX_TSS, (unsigned int)entries, (unsigned int)entries + sizeof (struct tss_entry) * TSS_ENTRY_SLOTS, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    cpu_set_tss(gdt_get_selector(GDT_INDEX_TSS));

}

