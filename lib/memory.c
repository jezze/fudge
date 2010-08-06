#include <lib/memory.h>

int memory_compare(void *dest, void *src, unsigned int count)
{

    unsigned char *dp = dest;
    unsigned char *sp = src;

    while (count--)
    {

        if (*dp++ != *sp++)
            return *dp - *sp;

    }

    return 0;

}

void *memory_copy(void *dest, void *src, unsigned int count)
{

    unsigned char *dp = dest;
    unsigned char *sp = src;

    while (count--)
        *dp++ = *sp++;

    return dest;

}

void *memory_set(void *dest, int value, unsigned int count)
{

    unsigned char *dp = dest;

    while (count--)
        *dp++ = value;

    return dest;

}

unsigned short *memory_setw(void *dest, unsigned short value, unsigned int count)
{

    unsigned short *dp = dest;

    while (count--)
        *dp++ = value;

    return dest;

}

