#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "uart.h"

static unsigned int buffer_getc(struct uart_buffer *self, char *buffer)
{

    if (self->head != self->tail)
    {

        buffer[0] = self->buffer[self->tail];
        self->tail = ((self->tail + 1) % self->size);

        return 1;

    }

    return 0;

}

static unsigned int buffer_putc(struct uart_buffer *self, char *buffer)
{

    if ((self->head + 1) % self->size != self->tail)
    {

        self->buffer[self->head] = buffer[0];
        self->head = ((self->head + 1) % self->size);

        return 1;

    }

    return 0;

}

static void handle_irq(struct base_device *device)
{

    struct uart_device *uartDevice = (struct uart_device *)device;
    struct uart_driver *driver = (struct uart_driver *)device->driver;
    char c = uartDevice->read(uartDevice);

    driver->buffer.putc(&driver->buffer, &c);

}

static void attach(struct base_device *device)
{

    struct uart_device *uartDevice = (struct uart_device *)device;

    pic_set_routine(device->irq, device, handle_irq);
    io_outb(uartDevice->port + UART_IER, 0x00);
    io_outb(uartDevice->port + UART_LCR, 0x80);
    io_outb(uartDevice->port + UART_THR, 0x03);
    io_outb(uartDevice->port + UART_IER, 0x00);
    io_outb(uartDevice->port + UART_LCR, 0x03);
    io_outb(uartDevice->port + UART_FCR, 0xC7);
    io_outb(uartDevice->port + UART_MCR, 0x0B);
    io_outb(uartDevice->port + UART_IER, 0x01);

}

static unsigned int check(struct base_device *device)
{

    return device->type == UART_DEVICE_TYPE;

}

void uart_init_driver(struct uart_driver *driver)
{

    memory_clear(driver, sizeof (struct uart_driver));
    base_init_driver(&driver->base, "uart", 0, check, attach);

    driver->buffer.size = 256;
    driver->buffer.getc = buffer_getc;
    driver->buffer.putc = buffer_putc;

}

