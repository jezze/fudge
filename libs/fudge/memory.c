#include "memory.h"

void memory_clear(void *out, unsigned int count)
{

    char *op = out;

    for (; count; count--)
        *op++ = 0;

}

void memory_copy(void *out, const void *in, unsigned int count)
{

    char *op = out;
    const char *ip = in;

    for (; count; count--)
        *op++ = *ip++;

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

