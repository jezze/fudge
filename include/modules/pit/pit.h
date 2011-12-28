#ifndef MODULES_PIT_H
#define MODULES_PIT_H

#define PIT_HERTZ     1193180
#define PIT_FREQUENCY 100

#define PIT_DEVICE_TYPE 0x2202

struct pit_device
{

    struct modules_device base;
    unsigned short divisor;
    unsigned short jiffies;

};

extern void pit_device_init(struct pit_device *device);

#endif

