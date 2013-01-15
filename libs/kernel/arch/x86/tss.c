#include <fudge/kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "tss.h"

static struct tss_entry entries[TSS_ENTRY_SLOTS];

void tss_set_entry(enum tss_index index, unsigned int selector, unsigned int stack)
{

    entries[index].ss0 = selector;
    entries[index].esp0 = stack;

}

void tss_setup(unsigned short selector, unsigned int stack)
{

    memory_clear(&entries, sizeof (struct tss_entry) * TSS_ENTRY_SLOTS);
    tss_set_entry(TSS_INDEX_NULL, selector, stack);
    gdt_set_entry(GDT_INDEX_TSS, (unsigned int)entries, (unsigned int)entries + sizeof (struct tss_entry) * TSS_ENTRY_SLOTS, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);
    cpu_set_tss(gdt_get_selector(GDT_INDEX_TSS));

}

