#include <kernel/arch/x86/io.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/uart/uart.h>

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

static void handle_irq(struct modules_device *self)
{

    struct uart_device *device = (struct uart_device *)self;
    struct uart_driver *driver = (struct uart_driver *)self->driver;

    char c = device->read(device);

    driver->buffer.putc(&driver->buffer, &c);

}

void uart_device_init(struct uart_device *device, unsigned int port, unsigned int irq)
{

    modules_device_init(&device->base, UART_DEVICE_TYPE, "uart");

    device->port = port;
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

    irq_register_routine(irq, &device->base, handle_irq);

}

