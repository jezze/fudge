#include <string.h>

void *memcpy(void *out, const void *in, size_t count)
{

    unsigned char *op = out;
    const unsigned char *ip = in;
    size_t i;

    for (i = 0; i < count; i++)
        op[i] = ip[i];

    return out;

}

void *memmove(void *out, const void *in, size_t count)
{

    unsigned char *op = out;
    const unsigned char *ip = in;
    size_t i;

    if (op < ip)
    {

        for (i = 0; i < count; i++)
            op[i] = ip[i];

    }

    else
    {

        for (i = count; i > 0; i--)
            op[i - 1] = ip[i - 1];

    }

    return out;

}

void *memset(void *out, int value, size_t count)
{

    unsigned char *op = out;
    size_t i;

    for (i = 0; i < count; i++)
        op[i] = value;

    return out;

}

int memcmp(const void *in1, const void *in2, size_t count)
{

    const unsigned char *ip1 = in1;
    const unsigned char *ip2 = in2;
    size_t i;

    for (i = 0; i < count; i++)
    {

        if (ip1[i] != ip2[i])
            return ip1[i] - ip2[i];

    }

    return 0;

}

size_t strlen(const char *in)
{

    size_t i = 0;

    while (in[i])
        i++;

    return i;

}

