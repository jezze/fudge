#include <fudge.h>
#include "tss.h"

void tss_setdescriptor(struct tss_pointer *pointer, unsigned short index, unsigned long stack)
{

    pointer->descriptors[index].rsp0.low = stack;
    pointer->descriptors[index].rsp0.high = stack >> 32;
    pointer->descriptors[index].iopb = sizeof (struct tss_descriptor);

}

void tss_init(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors)
{

    pointer->descriptors = descriptors;
    pointer->limit = sizeof (struct tss_descriptor) * count;

    buffer_clear(descriptors, sizeof (struct tss_descriptor) * count);

}
