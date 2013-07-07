#include <fudge/kernel.h>
#include "tss.h"

#define TSS_ENTRIES                     1

static struct tss_entry entries[TSS_ENTRIES];
static struct tss_pointer pointer;

void tss_set_entry(struct tss_pointer *p, enum tss_index index, unsigned int selector, unsigned int stack)
{

    p->base[index].ring0.ss = selector;
    p->base[index].ring0.esp = stack;

}

struct tss_pointer *tss_setup_pointer()
{

    pointer.base = entries;
    pointer.limit = sizeof (struct tss_entry) * TSS_ENTRIES;

    return &pointer;

}

