#include <lib/memory.h>

int memory_compare(void *dest, void *src, unsigned int count)
{

    unsigned char *dp = dest;
    unsigned char *sp = src;

    for (; count--; dp++, sp++)
    {

        if (*dp != *sp)
            return *dp - *sp;

    }

    return 0;

}

void *memory_copy(void *dest, void *src, unsigned int count)
{

    unsigned char *dp = dest;
    unsigned char *sp = src;

    for (; count--; dp++, sp++)
        *dp = *sp;

    return dest;

}

void *memory_replace(void *dest, char value1, char value2, unsigned int count)
{

    unsigned char *dp = dest;

    for (; count--; dp++)
    {

        if (*dp == value1)
            *dp = value2;

    }

    return dest;

}


void *memory_set(void *dest, char value, unsigned int count)
{

    unsigned char *dp = dest;

    for (; count--; dp++)
        *dp = value;

    return dest;

}

unsigned short *memory_setw(void *dest, short value, unsigned int count)
{

    unsigned short *dp = dest;

    for (; count--; dp++)
        *dp = value;

    return dest;

}

