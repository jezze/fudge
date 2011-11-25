#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/serial/serial.h>

static struct serial_device serialDevice1;

static unsigned int serial_buffer_getc(struct serial_buffer *self, char *buffer)
{

    if (self->head != self->tail)
    {

        buffer[0] = self->buffer[self->tail];
        self->tail = ((self->tail + 1) % self->size);

        return 1;

    }

    return 0;

}

static unsigned int serial_buffer_putc(struct serial_buffer *self, char *buffer)
{

    if ((self->head + 1) % self->size != self->tail)
    {

        self->buffer[self->head] = buffer[0];
        self->head = ((self->head + 1) % self->size);

        return 1;

    }

    return 0;

}

static char serial_device_read(struct serial_device *self)
{

    while (!(io_inb(self->port + 5) & 0x01));

    return io_inb(self->port);

}

static void serial_device_write(struct serial_device *self, char c)
{

    while (!(io_inb(self->port + 5) & 0x20));

    io_outb(self->port, c);

}

static void serial_handle_irq()
{

    char c = serialDevice1.read(&serialDevice1);

    serialDevice1.buffer.putc(&serialDevice1.buffer, &c);

}

void serial_device_init(struct serial_device *device, unsigned int port)
{

    modules_device_init(&device->base, SERIAL_DEVICE_TYPE);
    device->buffer.size = 256;
    device->buffer.head = 0;
    device->buffer.tail = 0;
    device->buffer.getc = serial_buffer_getc;
    device->buffer.putc = serial_buffer_putc;
    device->port = port;
    device->read = serial_device_read;
    device->write = serial_device_write;

    io_outb(device->port + 1, 0x00);
    io_outb(device->port + 3, 0x80);
    io_outb(device->port + 0, 0x03);
    io_outb(device->port + 1, 0x00);
    io_outb(device->port + 3, 0x03);
    io_outb(device->port + 2, 0xC7);
    io_outb(device->port + 4, 0x0B);
    io_outb(device->port + 1, 0x01);

}

void init()
{

    serial_device_init(&serialDevice1, SERIAL_COM1);

    kernel_register_irq(0x04, serial_handle_irq);

    modules_register_device(&serialDevice1.base);

}

void destroy()
{

    kernel_unregister_irq(0x04);

    modules_unregister_device(&serialDevice1.base);

}

