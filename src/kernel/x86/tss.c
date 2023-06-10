#include <fudge.h>
#include "tss.h"

void tss_setdescriptor(struct tss_pointer *pointer, unsigned char index, unsigned int selector, unsigned int stack)
{

    pointer->descriptors[index].ring0.ss = selector;
    pointer->descriptors[index].ring0.esp = stack;

}

void tss_init(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors)
{

    pointer->descriptors = descriptors;
    pointer->limit = sizeof (struct tss_descriptor) * count;

    buffer_clear(descriptors, sizeof (struct tss_descriptor) * count);

}

