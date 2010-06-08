#include <lib/types.h>
#include <lib/memory.h>

void *memory_copy(void *dest, const void *src, int32_t count)
{

    const int8_t *sp = (const int8_t *)src;
    int8_t *dp = (int8_t *)dest;

    for (; count != 0; count--)
        *dp++ = *sp++;

    return dest;

}

void *memory_set(void *dest, int8_t val, int32_t count)
{

    int8_t *temp = (int8_t *)dest;

    for (; count != 0; count--)
        *temp++ = val;

    return dest;

}

uint16_t *memory_setw(uint16_t *dest, uint16_t val, int32_t count)
{

    uint16_t *temp = (uint16_t *)dest;

    for (; count != 0; count--)
        *temp++ = val;

    return dest;

}

uint8_t inb(uint16_t port)
{

    uint8_t rv;

    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));

    return rv;

}

uint16_t inw(uint16_t port)
{

    uint16_t rv;

    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (port));

    return rv;

}

void outb(uint16_t port, uint8_t data)
{

    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));

}

