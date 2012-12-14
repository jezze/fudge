#include "memory.h"
#include "string.h"

unsigned int string_length(const char *in)
{

    unsigned int offset;

    for (offset = 0; in[offset]; offset++);

    return offset;

}

unsigned int string_read_num(const char *in, unsigned int base)
{

    const char *ip;
    unsigned int num = 0;

    for (ip = in; (*ip >= '0' && *ip <= '9') || (base > 10 && (*ip >= 'a' && *ip <= 'f')); ip++)
    {

        unsigned int offset = (*ip > '9') ? *ip - 'a' + 10 : *ip - '0';

        num = num * base + offset;

    }

    return num;

}

char *string_write_num(char *out, unsigned int num, unsigned int base)
{

    char buffer[32];
    unsigned int offset;

    if (!num)
        return memory_copy(out, "0", 2);

    memory_clear(buffer, 32);

    for (offset = 30; num && offset; --offset, num /= base)
        buffer[offset] = "0123456789abcdef"[num % base];

    return memory_copy(out, buffer + offset + 1, 32 - offset);

}

