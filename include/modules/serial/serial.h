#ifndef MODULES_SERIAL_H
#define MODULES_SERIAL_H

#define SERIAL_COM1 0x03F8
#define SERIAL_COM2 0x02F8
#define SERIAL_COM3 0x03E8
#define SERIAL_COM4 0x02E8

struct serial_device
{

    struct modules_device base;
    unsigned int port;
    void (*init)(struct serial_device *device);
    char (*read)(struct serial_device *device);
    void (*write)(struct serial_device *device, char c);

};

extern void serial_init();

#endif

