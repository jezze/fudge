#include <memory.h>
#include <modules.h>
#include <arch/x86/io/io.h>
#include <arch/x86/uart/uart.h>

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

void uart_device_init(struct uart_device *device, unsigned int port, unsigned int irq)
{

    memory_clear(device, sizeof (struct uart_device));

    modules_device_init(&device->base, UART_DEVICE_TYPE, "uart");

    device->port = port;
    device->irq = irq;
    device->read = read;
    device->write = write;

    io_outb(device->port + UART_IER, 0x00);
    io_outb(device->port + UART_LCR, 0x80);
    io_outb(device->port + UART_THR, 0x03);
    io_outb(device->port + UART_IER, 0x00);
    io_outb(device->port + UART_LCR, 0x03);
    io_outb(device->port + UART_FCR, 0xC7);
    io_outb(device->port + UART_MCR, 0x0B);
    io_outb(device->port + UART_IER, 0x01);

}

