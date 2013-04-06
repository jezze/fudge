#include <fudge/memory.h>
#include "posix.h"

void *memcpy(void *out, const void *in, unsigned int count)
{

    memory_copy(out, in, count);

    return out;

}

