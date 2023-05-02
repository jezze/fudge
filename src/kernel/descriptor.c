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

}

void descriptor_reset(struct descriptor *descriptor)
{

    descriptor->service = 0;
    descriptor->id = 0;

}

void descriptor_init(struct descriptor *descriptor)
{

    descriptor->service = 0;
    descriptor->id = 0;

}

