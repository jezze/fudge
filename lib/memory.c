#include <lib/memory.h>

int memory_compare(const void *out, const void *in, unsigned int count)
{

    const char *op = out;
    const char *ip = in;

    for (; count; op++, ip++, count--)
    {

        if (*op != *ip)
            return *op - *ip;

    }

    return 0;

}

void *memory_copy(void *out, const void *in, unsigned int count)
{

    char *op = out;
    const char *ip = in;

    for (; count; op++, ip++, count--)
        *op = *ip;

    return out;

}

unsigned int memory_index(const void *in, char value, unsigned int count)
{

    const char *ip = in;

    for (; count; ip++, count--)
    {

        if (*ip == value)
            break;

    }

    return ip - (char *)in;

}

void *memory_replace(void *out, char value1, char value2, unsigned int count)
{

    char *op = out;

    for (; count; op++, count--)
    {

        if (*op == value1)
            *op = value2;

    }

    return out;

}


void *memory_set(void *out, char value, unsigned int count)
{

    char *op = out;

    for (; count; op++, count--)
        *op = value;

    return out;

}

short *memory_setw(void *out, short value, unsigned int count)
{

    short *op = out;

    for (; count; op++, count--)
        *op = value;

    return out;

}

