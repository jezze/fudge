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

static void uart_driver_start(struct modules_driver *self)
{

}

static void uart_driver_attach(struct modules_driver *self, struct modules_device *device)
{

    device->driver = self;

}

static unsigned int uart_driver_check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != UART_DEVICE_TYPE)
        return 0;

    return 1;

}

void uart_driver_init(struct uart_driver *driver)
{

    modules_driver_init(&driver->base, UART_DRIVER_TYPE);
    driver->base.start = uart_driver_start;
    driver->base.attach = uart_driver_attach;
    driver->base.check = uart_driver_check;
    driver->buffer.size = 256;
    driver->buffer.head = 0;
    driver->buffer.tail = 0;
    driver->buffer.getc = uart_buffer_getc;
    driver->buffer.putc = uart_buffer_putc;

}

