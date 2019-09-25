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

    while ((*out++ = *in++));

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

unsigned int ascii_wvalue(char *out, unsigned int count, int value, unsigned int base)
{

    char *current = out;
    int b = base;
    int num = value;
    int tmp = value;
    unsigned int i;

    for (i = 1; i < count; i++)
    {

        tmp = num;
        num /= b;
        *current++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp - num * b)];

        if (!num)
            break;

    }

    if (tmp < 0)
    {

        *current++ = '-';

        i++;

    }

    current--;

    while (out < current)
    {

        char c = *current;

        *current-- = *out;
        *out++ = c;

    }

    return i;

}

