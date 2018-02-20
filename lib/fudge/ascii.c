#include "define.h"
#include "memory.h"
#include "ascii.h"

unsigned int ascii_length(char *in)
{

    unsigned int length = 0;

    while (in[length] != '\0')
        length++;

    return length;

}

void ascii_copy(char *out, char *in)
{

    while (*in)
        *out++ = *in++;

}

unsigned int ascii_toint(unsigned char c)
{

    if (c >= 'a')
        return c - 'a' + 10;

    if (c >= 'A')
        return c - 'A' + 10;

    if (c >= '0')
        return c - '0';

    return 0;

}

unsigned int ascii_rvalue(char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + ascii_toint(in[i]);

    return value;

}

unsigned int ascii_wvalue(void *out, unsigned int count, unsigned int value, unsigned int base)
{

    unsigned int current = value / base;
    unsigned int i = 0;
    unsigned char *o = out;

    if (!count)
        return 0;

    if (current)
        i = ascii_wvalue(out, count - 1, current, base);

    o[i] = "0123456789abcdef"[value % base];

    return ++i;

}

unsigned int ascii_wzerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[FUDGE_NSIZE];
    unsigned int bcount = ascii_wvalue(num, FUDGE_NSIZE, value, base);

    memory_write(out, count, "00000000000000000000000000000000", padding, offset);
    memory_write(out, count, num, bcount, offset + padding - bcount);

    return padding;

}

unsigned int ascii_dname(char *out, unsigned int ocount, char *in, unsigned int icount, char c)
{

    unsigned int i = 0;
    unsigned int p = 0;

    if (!icount)
        return 0;

    if (in[icount - 1] == c)
        icount--;

    for (i = 0; i < icount; i++)
    {

        if (in[i] == c)
            p = i;

    }

    memory_copy(out, in, p);

    return p;

}

unsigned int ascii_fname(char *out, unsigned int ocount, char *in, unsigned int icount, char c)
{

    unsigned int i = 0;
    unsigned int p = 0;

    if (!icount)
        return 0;

    if (in[icount - 1] == c)
        icount--;

    for (i = 0; i < icount; i++)
    {

        if (in[i] == c)
            p = i + 1;

    }

    memory_copy(out, in + p, i - p);

    return i - p;

}

