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

unsigned int ascii_fromint(void *out, unsigned int count, unsigned int value, unsigned int base)
{

    unsigned int current = value / base;
    unsigned int i = 0;
    unsigned char *o = out;

    if (!count)
        return 0;

    if (current)
        i = ascii_fromint(out, count - 1, current, base);

    o[i] = "0123456789abcdef"[value % base];

    return ++i;

}

unsigned int ascii_rvalue(char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + ascii_toint(in[i]);

    return value;

}

unsigned int ascii_wvalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int offset)
{

    char num[FUDGE_NSIZE];

    return memory_write(out, count, num, ascii_fromint(num, FUDGE_NSIZE, value, base), offset);

}

unsigned int ascii_wzerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[FUDGE_NSIZE];
    unsigned int bcount = ascii_fromint(num, FUDGE_NSIZE, value, base);

    memory_write(out, count, "00000000000000000000000000000000", padding, offset);
    memory_write(out, count, num, bcount, offset + padding - bcount);

    return padding;

}

unsigned int ascii_count(char *in, unsigned int count, char c)
{

    unsigned int total = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (in[i] == c)
            total++;

    }

    return total;

}

