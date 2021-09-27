#include <fudge.h>
#include "descriptor.h"

unsigned int descriptor_check(struct descriptor *descriptor)
{

    return descriptor->service && descriptor->id;

}

