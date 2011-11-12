#ifndef MODULES_MOUSE_H
#define MODULES_MOUSE_H

#define MOUSE_DEVICE_TYPE 59

struct mouse_device
{

    struct modules_device base;
    unsigned char cycle;
    char byte[3];
    char x;
    char y;

};

extern void mouse_device_init(struct mouse_device *device);

#endif

