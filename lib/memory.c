#include <lib/memory.h>

void *memory_copy(void *dest, const void *src, unsigned int count)
{

    char *dp = dest;
    const char *sp = src;

    while (count-- != 0)
        *dp++ = *sp++;

    return dest;

}

void *memory_set(void *dest, int value, unsigned int count)
{

    unsigned char *dp = dest;

    while (count-- != 0)
        *dp++ = value;

    return dest;

}

unsigned short *memory_setw(unsigned short *dest, unsigned short value, unsigned int count)
{

    unsigned short *dp = dest;

    while (count-- != 0)
        *dp++ = value;

    return dest;

}

