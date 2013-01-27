#include <fudge/kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "tss.h"

static struct tss_entry entries[TSS_ENTRY_SLOTS];

void tss_set_entry(struct tss_entry *entry, unsigned int selector, unsigned int stack)
{

    entry->ss0 = selector;
    entry->esp0 = stack;

}

struct tss_entry *tss_setup()
{

    memory_clear(&entries, sizeof (struct tss_entry) * TSS_ENTRY_SLOTS);

    return entries;

}

