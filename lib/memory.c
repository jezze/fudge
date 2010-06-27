#include <lib/types.h>
#include <lib/memory.h>

void *memory_copy(void *dest, const void *src, unsigned int count)
{

    const int8_t *sp = (const int8_t *)src;
    int8_t *dp = (int8_t *)dest;

    for (; count != 0; count--)
        *dp++ = *sp++;

    return dest;

}

void *memory_set(void *dest, int8_t value, unsigned int count)
{

    int8_t *temp = (int8_t *)dest;

    for (; count != 0; count--)
        *temp++ = value;

    return dest;

}

uint16_t *memory_setw(uint16_t *dest, uint16_t value, unsigned int count)
{

    uint16_t *temp = (uint16_t *)dest;

    for (; count != 0; count--)
        *temp++ = value;

    return dest;

}

