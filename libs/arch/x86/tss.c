#include <fudge.h>
#include "tss.h"

void tss_setdescriptor(struct tss_pointer *pointer, enum tss_index index, unsigned int selector, unsigned int stack)
{

    pointer->descriptors[index].ring0.ss = selector;
    pointer->descriptors[index].ring0.esp = stack;

}

void tss_initpointer(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors)
{

    pointer->descriptors = descriptors;
    pointer->limit = sizeof (struct tss_descriptor) * count;

}

