#include <fudge/kernel.h>
#include "tss.h"

void tss_set_entry(struct tss_pointer *pointer, enum tss_index index, unsigned int selector, unsigned int stack)
{

    pointer->base[index].ring0.ss = selector;
    pointer->base[index].ring0.esp = stack;

}

void tss_init_pointer(struct tss_pointer *pointer, unsigned int count, struct tss_entry *entries)
{

    pointer->base = entries;
    pointer->limit = sizeof (struct tss_entry) * count;

}

