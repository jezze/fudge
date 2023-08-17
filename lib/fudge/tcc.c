#include "tcc.h"

void *memmove(void *dst, void *src, unsigned int count)
{

    unsigned char *op = dst;
    unsigned char *ip = src;

    while (count--)
        *op++ = *ip++;

    return dst;

}

void *memset(void *dst, int c, unsigned int count)
{

    unsigned char *op = dst;

    while (count--)
        *op++ = c;

    return dst;

}

