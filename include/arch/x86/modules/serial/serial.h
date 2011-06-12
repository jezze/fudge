#ifndef ARCH_SERIAL_H
#define ARCH_SERIAL_H

#define SERIAL_COM1 0x03F8
#define SERIAL_COM2 0x02F8
#define SERIAL_COM3 0x03E8
#define SERIAL_COM4 0x02E8

struct modules_serial_device
{

    struct modules_module base;
    unsigned int port;
    unsigned int (*read)(struct modules_serial_device *device, char *buffer, unsigned int count);
    unsigned int (*write)(struct modules_serial_device *device, char *buffer, unsigned int count);
    struct file_node node;

};

extern void serial_init();

#endif

