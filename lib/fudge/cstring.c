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

unsigned int cstring_lengthzero(char *in)
{

    return cstring_length(in) + 1;

}

void cstring_copy(char *out, char *in)
{

    while ((*out++ = *in++));

}

unsigned int cstring_match(char *in1, char *in2)
{

    unsigned int n1 = cstring_lengthzero(in1);
    unsigned int n2 = cstring_lengthzero(in2);

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

unsigned int cstring_readvalue(char *in, unsigned int count, unsigned int base)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
        value = value * base + cstring_toint(in[i]);

    return value;

}

unsigned int cstring_write(char *out, unsigned int count, char *in, unsigned int offset)
{

    return buffer_write(out, count, in, cstring_length(in), offset);

}

unsigned int cstring_writezero(char *out, unsigned int count, unsigned int offset)
{

    if (offset < count)
    {

        out[offset] = '\0';

        return 1;

    }

    else
    {

        out[count - 1] = '\0';

        return 0;

    }

}

unsigned int cstring_writevalue(char *out, unsigned int count, int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[1024];

    return buffer_write(out, count, num, writevalue(num, 1024, value, base, padding), offset);

}

unsigned int cstring_writefmt(void *out, unsigned int count, char *fmt, unsigned int nargs, void **args, unsigned int offset)
{

    unsigned int length = cstring_length(fmt);
    unsigned int escaped = 0;
    unsigned int cargs = 0;
    unsigned int base = 10;
    unsigned int padding = 0;
    unsigned int start = offset;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        char q = fmt[i];

        if (escaped)
        {

            char num[64];
            unsigned char c;
            unsigned short h;
            unsigned int u;
            int i;
            char *s;

            switch (q)
            {

            case '0':
                padding = 0;

                break;

            case '1':
                padding = 1;

                break;

            case '2':
                padding = 2;

                break;

            case '4':
                padding = 4;

                break;

            case '8':
                padding = 8;

                break;

            case 'D':
                base = 10;

                break;

            case 'H':
                base = 16;

                break;

            case 'c':
                c = *((unsigned char *)args[cargs++]);
                offset += buffer_write(out, count, num, writevalue(num, 64, c, base, padding), offset);
                escaped = 0;

                break;

            case 'h':
                h = *((unsigned short *)args[cargs++]);
                offset += buffer_write(out, count, num, writevalue(num, 64, h, base, padding), offset);
                escaped = 0;

                break;

            case 'u':
                u = *((unsigned int *)args[cargs++]);
                offset += buffer_write(out, count, num, writevalue(num, 64, u, base, padding), offset);
                escaped = 0;

                break;

            case 'i':
                i = *((int *)args[cargs++]);
                offset += buffer_write(out, count, num, writevalue(num, 64, i, base, padding), offset);
                escaped = 0;

                break;

            case 's':
                s = ((char *)args[cargs++]);
                offset += buffer_write(out, count, s, cstring_length(s), offset);
                escaped = 0;

                break;

            case 'w':
                s = ((char *)args[cargs++]);
                u = *((unsigned int *)args[cargs++]);
                offset += buffer_write(out, count, s, u, offset);
                escaped = 0;

                break;

            default:
                escaped = 0;

                break;

            }

        }

        else
        {

            switch (q)
            {

            case '%':
                escaped = 1;

                break;

            default:
                offset += buffer_write(out, count, &q, 1, offset);

                break;

            }

        }

    }

    return offset - start;

}

