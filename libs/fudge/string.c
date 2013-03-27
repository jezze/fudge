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

unsigned int string_write_num(void *out, unsigned int count, unsigned int num, unsigned int base)
{

    char buffer[32];
    unsigned int c;

    if (!num)
        return memory_write(out, count, "0", 2, 0);

    memory_clear(buffer, 32);

    for (c = 30; num && c; --c, num /= base)
        buffer[c] = "0123456789abcdef"[num % base];

    return memory_write(out, count, buffer + c + 1, 32 - c, 0);

}

unsigned int string_write_format(void *out, unsigned int count, const char *format, const void **args)
{

    char *o = out;
    unsigned int c = 0;
    unsigned int j = 0;
    unsigned int i;

    for (i = 0; i < string_length(format) + 1; i++)
    {

        if (format[i] != '%')
        {

            c += memory_write(o, count, format + i, 1, c);

            continue;

        }

        switch (format[i + 1])
        {

            case 'c':

                c += memory_write(o, count, args[j], 1, c);

                break;

            case 's':

                c += memory_write(o, count, args[j], string_length(args[j]), c);

                break;

            case 'u':

                c += string_write_num(o + c, count - c, *((unsigned int *)args[j]), 10);

                break;

            case 'd':

                c += string_write_num(o + c, count - c, *((int *)args[j]), 10);

                break;

            case 'x':

                c += string_write_num(o + c, count - c, *((unsigned int *)args[j]), 16);

                break;

        }

        i++;
        j++;

    }

    return c;

}

