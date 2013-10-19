#include "memory.h"
#include "ascii.h"

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

unsigned char ascii_tochar(unsigned int value, unsigned int base)
{

    return "0123456789abcdef"[value % base];

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
    unsigned int index;

    if (!value)
        return memory_write(out, count, "0", 1, offset);

    for (index = 31; value && index; --index, value /= base)
        buffer[index] = ascii_tochar(value, base);

    return memory_write(out, count, buffer + index + 1, 31 - index, offset);

}

unsigned int ascii_write_zerovalue(void *out, unsigned int count, unsigned int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char buffer[32];
    unsigned int index;

    if (padding > 32)
        return 0;

    for (index = 0; index < 32; index++)
        buffer[index] = '0';

    if (!value)
        return memory_write(out, count, buffer, padding, offset);

    for (index = 31; value && index; --index, value /= base)
        buffer[index] = ascii_tochar(value, base);

    if (31 - index < padding)
        index = 31 - padding;

    return memory_write(out, count, buffer + index + 1, 31 - index, offset);

}

