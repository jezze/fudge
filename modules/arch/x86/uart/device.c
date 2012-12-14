#include <fudge/memory.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "uart.h"

static char read(struct uart_device *self)
{

    while (!(io_inb(self->port + UART_LSR) & 0x01));

    return io_inb(self->port);

}

static void write(struct uart_device *self, char c)
{

    while (!(io_inb(self->port + UART_LSR) & 0x20));

    io_outb(self->port, c);

}

void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq)
{

    memory_clear(device, sizeof (struct uart_device));

    base_init_device(&device->base, UART_DEVICE_TYPE, "uart");

    device->port = port;
    device->irq = irq;
    device->read = read;
    device->write = write;

}

