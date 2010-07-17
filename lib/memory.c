#include <lib/memory.h>

void *memory_copy(void *dest, const void *src, unsigned int count)
{

    const char *sp = (const char *)src;
    char *dp = (char *)dest;

    for (; count != 0; count--)
        *dp++ = *sp++;

    return dest;

}

void *memory_set(void *dest, char value, unsigned int count)
{

    char *temp = (char *)dest;

    for (; count != 0; count--)
        *temp++ = value;

    return dest;

}

unsigned short *memory_setw(unsigned short *dest, unsigned short value, unsigned int count)
{

    unsigned short *temp = (unsigned short *)dest;

    for (; count != 0; count--)
        *temp++ = value;

    return dest;

}

