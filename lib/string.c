#include <lib/memory.h>
#include <lib/string.h>

char *string_find(const char *in, const char *str)
{

    char c = *str++;

    if (!c)
        return (char *)in;

    unsigned int len = string_length(str);

    do
    {

        char sc;

        do
        {

            sc = *in++;

            if (!sc)
                return (char *)0;

        }
        while (sc != c);

    }
    while (memory_compare(in, str, len));

    return (char *)(in - 1);

}

unsigned int string_length(const char *in)
{

    unsigned int i;

    for (i = 0; in[i]; i++);

    return i;

}

unsigned int string_read_num(const char *in, unsigned int base)
{

    const char *ip = in;
    int num = 0;

    while ((*ip >= '0' && *ip <= '9') || (*ip >= 'a' && *ip <= 'f'))
    {

        if ((*ip >= '0' && *ip <= '9'))
            num = num * base + *ip++ - '0';

        if ((*ip >= 'a' && *ip <= 'f'))
            num = num * base + *ip++ - 'a' + 10;

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

