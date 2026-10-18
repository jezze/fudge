#include "../buffer.h"

void buffer_copy(void *out, void *in, unsigned int count)
{

    unsigned char *op = out;
    unsigned char *ip = in;

    while (count--)
        *op++ = *ip++;

}

void buffer_clear(void *out, unsigned int count)
{

    unsigned char *op = out;

    while (count--)
        *op++ = 0;

}

