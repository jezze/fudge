#include "buffer.h"
#include "ascii.h"

unsigned int ascii_length(char *in)
{

    unsigned int length = 0;

    while (in[length] != '\0')
        length++;

    return length;

}

unsigned int ascii_lengthz(char *in)
{

    return ascii_length(in) + 1;

}

void ascii_copy(char *out, char *in)
{

    while ((*out++ = *in++));

}

unsigned int ascii_match(char *in1, char *in2)
{

    unsigned int n1 = ascii_lengthz(in1);
    unsigned int n2 = ascii_lengthz(in2);

    return buffer_match(in1, in2, (n1 < n2) ? n1 : n2);

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

unsigned int ascii_wvalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding)
{

    char *current = out;
    int b = base;
    int num = value;
    unsigned int i;

    for (i = 1; i < count; i++)
    {

        *current++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + num % b];

        num /= b;

        if (!num && i >= padding)
            break;

    }

    if (value < 0)
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

