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

void *memory_find(const void *in1, const void *in2, unsigned int count1, unsigned int count2)
{

    if (count2 > count1)
        return 0;

    const char *ip = in1;
    unsigned int count = count1 - count2;

    for (; count; count--)
    {

        if (memory_compare(ip, in2, count2))
            return (void *)ip;

        ip++;

    }

    return 0;

}

