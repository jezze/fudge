#include <lib/memory.h>

void *memory_clear(void *out, unsigned int count)
{

    char *op = out;

    for (; count; count--)
        *op++ = 0;

    return out;

}

int memory_compare(const void *in1, const void *in2, unsigned int count)
{

    const char *ip1 = in1;
    const char *ip2 = in2;

    for (; count; count--)
    {

        if (*ip1++ != *ip2++)
            return 0;

    }

    return 1;

}

void *memory_copy(void *out, const void *in, unsigned int count)
{

    char *op = out;
    const char *ip = in;

    for (; count; count--)
        *op++ = *ip++;

    return out;

}

unsigned int memory_index(const void *in, char value, unsigned int count)
{

    const char *ip = in;

    for (; count; count--)
    {

        if (*ip++ == value)
            break;

    }

    return ip - (char *)in;

}

