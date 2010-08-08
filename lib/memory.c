#include <lib/memory.h>

int memory_compare(const void *dest, const void *src, unsigned int count)
{

    const char *dp = dest;
    const char *sp = src;

    for (; count; dp++, sp++, count--)
    {

        if (*dp != *sp)
            return *dp - *sp;

    }

    return 0;

}

void *memory_copy(void *dest, const void *src, unsigned int count)
{

    char *dp = dest;
    const char *sp = src;

    for (; count; dp++, sp++, count--)
        *dp = *sp;

    return dest;

}

unsigned int memory_index(const void *dest, char value, unsigned int count)
{

    const char *dp = dest;

    for (; count; dp++, count--)
    {

        if (*dp == value)
            break;

    }

    return dp - (char *)dest;

}

void *memory_replace(void *dest, char value1, char value2, unsigned int count)
{

    char *dp = dest;

    for (; count; dp++, count--)
    {

        if (*dp == value1)
            *dp = value2;

    }

    return dest;

}


void *memory_set(void *dest, char value, unsigned int count)
{

    char *dp = dest;

    for (; count; dp++, count--)
        *dp = value;

    return dest;

}

short *memory_setw(void *dest, short value, unsigned int count)
{

    short *dp = dest;

    for (; count; dp++, count--)
        *dp = value;

    return dest;

}

