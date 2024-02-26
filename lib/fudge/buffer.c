#include "buffer.h"

void buffer_clear(void *out, unsigned int count)
{

    unsigned char *op = out;

    while (count--)
        *op++ = 0;

}

void buffer_copy(void *out, void *in, unsigned int count)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    while (count--)
        *op++ = *ip++;

}

unsigned int buffer_eachbyte(void *in, unsigned int count, unsigned char value, unsigned int offset)
{

    unsigned char *ip = in;

    while (offset < count)
    {

        if (ip[offset] == value)
            return offset + 1;

        offset++;

    }

    return 0;

}

unsigned int buffer_firstbyte(void *in, unsigned int count, unsigned char value)
{

    return buffer_eachbyte(in, count, value, 0);

}

unsigned int buffer_lastbyte(void *in, unsigned int count, unsigned char value)
{

    unsigned int offset = 0;
    unsigned int last = 0;

    while ((offset = buffer_eachbyte(in, count, value, offset)))
        last = offset;

    return last;

}

unsigned int buffer_countbyte(char *in, unsigned int count, unsigned char value)
{

    unsigned int offset = 0;
    unsigned int total = 0;

    while ((offset = buffer_eachbyte(in, count, value, offset)))
        total++;

    return total;

}

unsigned int buffer_findbyte(void *in, unsigned int count, unsigned char value)
{

    unsigned char *ip = in;
    unsigned int offset;

    for (offset = 0; offset < count; offset++)
    {

        if (ip[offset] == value)
            return offset;

    }

    return offset;

}

unsigned int buffer_match(void *in1, void *in2, unsigned int count)
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

unsigned int buffer_read(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    if (offset >= icount)
        return 0;

    if (ocount > icount - offset)
        ocount = icount - offset;

    buffer_copy(op, ip + offset, ocount);

    return ocount;

}

unsigned int buffer_write(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    if (offset >= ocount)
        return 0;

    if (icount > ocount - offset)
        icount = ocount - offset;

    buffer_copy(op + offset, ip, icount);

    return icount;

}

void *buffer_tindex(char *in, unsigned int count, char c, unsigned int index)
{

    if (index)
    {

        unsigned int n = 0;
        unsigned int i;

        for (i = 0; i < count; i++)
        {

            if (in[i] == c)
            {

                if (++n == index)
                    return (i == count - 1) ? 0 : in + i + 1;

            }

        }

        return 0;

    }

    else
    {

        return in;

    }

}

