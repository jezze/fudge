#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/io/io.h>
#include <modules/serial/serial.h>

struct serial_device serialDevice1;

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

static unsigned int serial_device_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!serialDevice1.buffer.getc(&serialDevice1.buffer, buffer + i))
            break;

    }

    return count;

}

static unsigned int serial_device_node_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    char *b = (char *)buffer;

    unsigned int i;

    for (i = 0; i < count; i++)
        serialDevice1.write(&serialDevice1, b[i]);

    return i;

}

static void serial_handler()
{

    char c = serialDevice1.read(&serialDevice1);

    serialDevice1.buffer.putc(&serialDevice1.buffer, &c);

}

void serial_device_init(struct serial_device *device, unsigned int port)
{

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = SERIAL_DEVICE_TYPE;
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

void serial_init()
{

    serial_device_init(&serialDevice1, SERIAL_COM1);

    kernel_register_irq(0x04, serial_handler);

    modules_register_device(&serialDevice1.base);

}

