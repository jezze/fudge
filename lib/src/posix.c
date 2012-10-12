#include <memory.h>
#include <posix.h>

void *memcpy(void *out, const void *in, unsigned int count)
{

    return memory_copy(out, in, count);

}

