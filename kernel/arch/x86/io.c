#include <kernel/arch/x86/io.h>

unsigned char inb(unsigned short port)
{

    unsigned char rv;

    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));

    return rv;

}

unsigned short inw(unsigned short port)
{

    unsigned short rv;

    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (port));

    return rv;

}

void outb(unsigned short port, unsigned char data)
{

    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));

}

