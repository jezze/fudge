#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/modules/serial/serial.h>
#include <kernel/vfs.h>

int serial_in_ready(unsigned short port)
{

    return io_inb(port + 5) & 0x01;

}

char serial_inb(unsigned short port)
{

    while (serial_in_ready(port) == 0);

    return io_inb(port);

}

unsigned int serial_read(unsigned short port, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        buffer[i] = serial_inb(port);

    return i;

}

int serial_out_ready(unsigned short port)
{

    return io_inb(port + 5) & 0x20;

}

void serial_outb(unsigned short port, char c)
{

    while (serial_out_ready(port) == 0);

    io_outb(port, c);

}

unsigned int serial_write(unsigned short port, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        serial_outb(port, buffer[i]);

    return i;

}

void serial_init()
{

    io_outb(SERIAL_COM1 + 1, 0x00);
    io_outb(SERIAL_COM1 + 3, 0x80);
    io_outb(SERIAL_COM1 + 0, 0x03);
    io_outb(SERIAL_COM1 + 1, 0x00);
    io_outb(SERIAL_COM1 + 3, 0x03);
    io_outb(SERIAL_COM1 + 2, 0xC7);
    io_outb(SERIAL_COM1 + 4, 0x0B);

}

