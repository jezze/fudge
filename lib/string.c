#include <lib/memory.h>
#include <lib/string.h>

unsigned int string_length(const char *in)
{

    unsigned int i;

    for (i = 0; in[i]; i++);

    return i;

}

unsigned int string_read_num(const char *in, unsigned int base)
{

    const char *ip;
    int num = 0;

    for (ip = in; (*ip >= '0' && *ip <= '9') || (base > 10 && (*ip >= 'a' && *ip <= 'f')); ip++)
    {

        unsigned int offset = (*ip > '9') ? *ip - 'a' + 10 : *ip - '0';

        num = num * base + offset;

    }

    return num;

}

char *string_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
        return memory_copy(out, "0", 2);

    char buffer[32];
    memory_clear(buffer, 32);

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return memory_copy(out, buffer + i + 1, string_length(buffer) + 1);

}

