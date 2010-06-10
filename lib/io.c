#include <lib/types.h>
#include <lib/io.h>

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

