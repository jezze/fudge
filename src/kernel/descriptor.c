#include <fudge.h>
#include "descriptor.h"

unsigned int descriptor_check(struct descriptor *descriptor)
{

    return descriptor->service && descriptor->id;

}

void descriptor_copy(struct descriptor *descriptor, struct descriptor *source)
{

    descriptor->service = source->service;
    descriptor->id = source->id;
    descriptor->offset = 0;
    descriptor->current = 0;
    descriptor->count = 0;

}

void descriptor_init(struct descriptor *descriptor)
{

    descriptor->service = 0;
    descriptor->id = 0;
    descriptor->offset = 0;
    descriptor->current = 0;
    descriptor->count = 0;

}

