#include "buffer.h"
#include "cstring.h"

static unsigned int writevalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding)
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

unsigned int cstring_length(char *in)
{

    unsigned int length = 0;

    while (*in++)
        length++;

    return length;

}

unsigned int cstring_lengthz(char *in)
{

    return cstring_length(in) + 1;

}

void cstring_copy(char *out, char *in)
{

    while ((*out++ = *in++));

}

unsigned int cstring_match(char *in1, char *in2)
{

    unsigned int n1 = cstring_lengthz(in1);
    unsigned int n2 = cstring_lengthz(in2);

    return buffer_match(in1, in2, (n1 < n2) ? n1 : n2);

}

unsigned int cstring_isalpha(char c)
{

    if (c >= 'a' && c <= 'z')
        return 1;

    if (c >= 'A' && c <= 'Z')
        return 1;

    return 0;

}

unsigned int cstring_isdigit(char c)
{

    if (c >= '0' && c <= '9')
        return 1;

    return 0;

}

unsigned int cstring_toint(char c)
{

    if (c >= 'a')
        return c - 'a' + 10;

    if (c >= 'A')
        return c - 'A' + 10;

    if (c >= '0')
        return c - '0';

    return 0;

}

unsigned int cstring_write(char *out, unsigned int count, char *in, unsigned int offset)
{

    return buffer_write(out, count, in, cstring_length(in), offset);

}

unsigned int cstring_writez(char *out, unsigned int count, char *in, unsigned int offset)
{

    unsigned int total = buffer_write(out, count, in, cstring_lengthz(in), offset);

    out[count - 1] = '\0';

    return total;

}

unsigned int cstring_writevalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[1024];

    return buffer_write(out, count, num, writevalue(num, 1024, value, base, padding), offset);

}

unsigned int cstring_readvalue(char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + cstring_toint(in[i]);

    return value;

}

