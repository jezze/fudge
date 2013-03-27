#include "memory.h"

void *memory_clear(void *out, unsigned int count)
{

    char *op = out;

    for (; count; count--)
        *op++ = 0;

    return out;

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

    const char *ip = in1;

    if (count2 > count1)
        return 0;

    count1 -= count2 + 1;

    for (; count1; count1--)
    {

        if (memory_match(ip, in2, count2))
            return (void *)ip;

        ip++;

    }

    return 0;

}

unsigned int memory_match(const void *in1, const void *in2, unsigned int count)
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

unsigned int memory_read(void *out, unsigned int ocount, const void *in, unsigned int icount, unsigned int offset)
{

    char *op = out;
    const char *ip = in;

    if (offset >= icount)
        return 0;

    if (ocount > icount - offset)
        ocount = icount - offset;

    ip += offset;

    for (offset = ocount; offset; offset--)
        *op++ = *ip++;

    return ocount;

}

unsigned int memory_write(void *out, unsigned int ocount, const void *in, unsigned int icount, unsigned int offset)
{

    char *op = out;
    const char *ip = in;

    if (offset >= ocount)
        return 0;

    if (icount > ocount - offset)
        icount = ocount - offset;

    op += offset;

    for (offset = icount; offset; offset--)
        *op++ = *ip++;

    return icount;

}

