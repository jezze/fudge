#include <fudge/kernel.h>
#include "tss.h"

void tss_set_descriptor(struct tss_pointer *pointer, enum tss_index index, unsigned int selector, unsigned int stack)
{

    pointer->descriptors[index].ring0.ss = selector;
    pointer->descriptors[index].ring0.esp = stack;

}

void tss_init_pointer(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors)
{

    pointer->descriptors = descriptors;
    pointer->limit = sizeof (struct tss_descriptor) * count;

}

