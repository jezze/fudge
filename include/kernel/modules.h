#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_DEVICE_TYPE_NULL   0
#define MODULES_DEVICE_TYPE_INPUT  1
#define MODULES_DEVICE_TYPE_OUTPUT 2

struct modules_device
{

    unsigned int type;

};

struct modules_device_stream
{

    struct modules_device base;
    unsigned int (*read)(char *buffer);
    unsigned int (*write)(char *buffer);

};

struct modules_device_binary_format
{

    struct modules_device base;
    unsigned int (*check)(void *address);

};

extern void modules_add_device(struct modules_device *device);
extern void modules_init();

#endif

