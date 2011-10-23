#ifndef MODULES_SERIAL_H
#define MODULES_SERIAL_H

#define SERIAL_COM1 0x03F8
#define SERIAL_COM2 0x02F8
#define SERIAL_COM3 0x03E8
#define SERIAL_COM4 0x02E8

#define SERIAL_DEVICE_TYPE 2

struct serial_buffer
{

    char buffer[256];
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    unsigned int (*getc)(struct serial_buffer *self, char *buffer);
    unsigned int (*putc)(struct serial_buffer *self, char *buffer);

};

struct serial_device
{

    struct modules_device base;
    struct serial_buffer buffer;
    unsigned int port;
    void (*init)(struct serial_device *self);
    char (*read)(struct serial_device *self);
    void (*write)(struct serial_device *self, char c);

};

extern void serial_init();

#endif

