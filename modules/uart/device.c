#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/uart/uart.h>

static unsigned int uart_buffer_getc(struct uart_buffer *self, char *buffer)
{

    if (self->head != self->tail)
    {

        buffer[0] = self->buffer[self->tail];
        self->tail = ((self->tail + 1) % self->size);

        return 1;

    }

    return 0;

}

static unsigned int uart_buffer_putc(struct uart_buffer *self, char *buffer)
{

    if ((self->head + 1) % self->size != self->tail)
    {

        self->buffer[self->head] = buffer[0];
        self->head = ((self->head + 1) % self->size);

        return 1;

    }

    return 0;

}

static char uart_device_read(struct uart_device *self)
{

    while (!(io_inb(self->port + UART_LSR) & 0x01));

    return io_inb(self->port);

}

static void uart_device_write(struct uart_device *self, char c)
{

    while (!(io_inb(self->port + UART_LSR) & 0x20));

    io_outb(self->port, c);

}

void uart_device_init(struct uart_device *device, unsigned int port)
{

    modules_device_init(&device->base, UART_DEVICE_TYPE);
    device->buffer.size = 256;
    device->buffer.head = 0;
    device->buffer.tail = 0;
    device->buffer.getc = uart_buffer_getc;
    device->buffer.putc = uart_buffer_putc;
    device->port = port;
    device->read = uart_device_read;
    device->write = uart_device_write;

    io_outb(device->port + UART_IER, 0x00);
    io_outb(device->port + UART_LCR, 0x80);
    io_outb(device->port + UART_THR, 0x03);
    io_outb(device->port + UART_IER, 0x00);
    io_outb(device->port + UART_LCR, 0x03);
    io_outb(device->port + UART_FCR, 0xC7);
    io_outb(device->port + UART_MCR, 0x0B);
    io_outb(device->port + UART_IER, 0x01);

}

