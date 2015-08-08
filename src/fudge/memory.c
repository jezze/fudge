#include "memory.h"

void memory_clear(void *out, unsigned int count)
{

    unsigned char *op = out;

    while (count--)
        *op++ = 0;

}

void memory_copy(void *out, void *in, unsigned int count)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    while (count--)
        *op++ = *ip++;

}

unsigned int memory_findbyte(void *in, unsigned int count, char value)
{

    unsigned char *ip = in;
    unsigned int offset;

    if (!count)
        return 0;

    for (offset = 1; offset < count && ip[offset - 1] != value; offset++);

    return offset;

}

unsigned int memory_match(void *in1, void *in2, unsigned int count)
{

    unsigned char *ip1 = in1;
    unsigned char *ip2 = in2;

    while (count--)
    {

        if (*ip1++ != *ip2++)
            return 0;

    }

    return 1;

}

unsigned int memory_read(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int size, unsigned int offset)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    icount = icount * size;
    ocount = ocount * size;

    if (offset >= icount)
        return 0;

    if (ocount > icount - offset)
        ocount = icount - offset;

    ip += offset * size;

    for (offset = ocount; offset; offset--)
        *op++ = *ip++;

    return ocount;

}

unsigned int memory_write(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int size, unsigned int offset)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    icount = icount * size;
    ocount = ocount * size;

    if (offset >= ocount)
        return 0;

    if (icount > ocount - offset)
        icount = ocount - offset;

    op += offset * size;

    for (offset = icount; offset; offset--)
        *op++ = *ip++;

    return icount;

}

