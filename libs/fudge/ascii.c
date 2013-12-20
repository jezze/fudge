#include "memory.h"
#include "ascii.h"

unsigned int ascii_length(const char *in)
{

    unsigned int offset;

    for (offset = 0; in[offset]; offset++);

    return offset;

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

unsigned int ascii_fromint(char *out, unsigned int count, unsigned int value, unsigned int base)
{

    unsigned int current = value / base;
    unsigned int i = 0;

    if (!count)
        return 0;

    if (current)
        i = ascii_fromint(out, count - 1, current, base);

    out[i] = "0123456789abcdef"[value % base];

    return ++i;

}

unsigned int ascii_read_value(const char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + ascii_toint(in[i]);

    return value;

}

unsigned int ascii_write_value(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int offset)
{

    char buffer[32];
    unsigned int bcount = ascii_fromint(buffer, 32, value, base);

    return memory_write(out, count, buffer, bcount, offset);

}

unsigned int ascii_write_zerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char buffer[32];
    unsigned int bcount = ascii_fromint(buffer, 32, value, base);
    unsigned int pcount = 0;
    unsigned int i;

    for (i = bcount; i < padding; i++)
        pcount += memory_write(out, count, "0", 1, offset);

    return pcount + memory_write(out, count, buffer, bcount, offset + pcount);

}

