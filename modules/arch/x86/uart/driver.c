#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/uart/uart.h>

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

static void handle_irq(struct modules_device *self)
{

    struct uart_device *device = (struct uart_device *)self;
    struct uart_driver *driver = (struct uart_driver *)self->driver;

    char c = device->read(device);

    driver->buffer.putc(&driver->buffer, &c);

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct uart_device *uartDevice = (struct uart_device *)device;

    irq_register_routine(uartDevice->irq, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != UART_DEVICE_TYPE)
        return 0;

    return 1;

}

void uart_driver_init(struct uart_driver *driver)
{

    memory_clear(driver, sizeof (struct uart_driver));

    modules_driver_init(&driver->base, UART_DRIVER_TYPE, "uart");

    driver->base.attach = attach;
    driver->base.check = check;
    driver->buffer.size = 256;
    driver->buffer.head = 0;
    driver->buffer.tail = 0;
    driver->buffer.getc = buffer_getc;
    driver->buffer.putc = buffer_putc;

}

