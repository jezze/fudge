#include <fudge/kernel.h>
#include "cpu.h"
#include "gdt.h"
#include "tss.h"

static struct tss_entry entries[TSS_ENTRY_SLOTS];
static struct tss_pointer pointer;

void tss_set_entry(struct tss_pointer *p, enum tss_index index, unsigned int selector, unsigned int stack)
{

    p->base[index].ss0 = selector;
    p->base[index].esp0 = stack;

}

struct tss_pointer *tss_setup_pointer()
{

    memory_clear(&entries, sizeof (struct tss_entry) * TSS_ENTRY_SLOTS);

    pointer.base = entries;

    return &pointer;

}

