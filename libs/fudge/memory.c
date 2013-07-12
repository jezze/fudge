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

unsigned int memory_write_number(void *out, unsigned int ocount, unsigned int num, unsigned int base, unsigned int offset)
{

    char buffer[32];
    unsigned int index;

    if (!num)
        return memory_write(out, ocount, "0", 1, offset);

    for (index = 31; num && index; --index, num /= base)
        buffer[index] = "0123456789abcdef"[num % base];

    return memory_write(out, ocount, buffer + index + 1, 31 - index, offset);

}

unsigned int memory_write_paddednumber(void *out, unsigned int ocount, unsigned int num, unsigned int base, unsigned int padding, unsigned int offset)
{

    char buffer[32];
    unsigned int index;

    if (padding > 32)
        return 0;

    for (index = 0; index < 32; index++)
        buffer[index] = '0';

    if (!num)
        return memory_write(out, ocount, buffer, padding, offset);

    for (index = 31; num && index; --index, num /= base)
        buffer[index] = "0123456789abcdef"[num % base];

    if (31 - index < padding)
        index = 31 - padding;

    return memory_write(out, ocount, buffer + index + 1, 31 - index, offset);

}

