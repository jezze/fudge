#ifndef MODULES_BGA_H
#define MODULES_BGA_H

#define BGA_DEVICE_TYPE 593

struct bga_device
{

    struct modules_device base;

};

extern void bga_device_init(struct bga_device *device);

#endif

