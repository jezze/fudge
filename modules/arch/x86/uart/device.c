#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <arch/x86/io/io.h>
#include "uart.h"

char uart_device_read(struct uart_device *device)
{

    while (!(io_inb(device->port + UART_LSR) & 0x01));

    return io_inb(device->port);

}

void uart_device_write(struct uart_device *device, char c)
{

    while (!(io_inb(device->port + UART_LSR) & 0x20));

    io_outb(device->port, c);

}

void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq)
{

    memory_clear(device, sizeof (struct uart_device));
    base_init_device(&device->base, UART_DEVICE_TYPE, irq, "uart", 0);

    device->port = port;

}

