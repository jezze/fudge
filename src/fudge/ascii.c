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

unsigned int ascii_rvalue(const char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + ascii_toint(in[i]);

    return value;

}

unsigned int ascii_wvalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int offset)
{

    unsigned char buffer[32];
    unsigned int bcount = ascii_fromint(buffer, 32, value, base);

    return memory_write(out, count, buffer, bcount, 1, offset);

}

unsigned int ascii_wzerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    unsigned char buffer[32];
    unsigned int bcount = ascii_fromint(buffer, 32, value, base);

    memory_write(out, count, "00000000000000000000000000000000", padding, 1, offset);
    memory_write(out, count, buffer, bcount, 1, offset + padding - bcount);

    return padding;

}

