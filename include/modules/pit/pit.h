#ifndef MODULES_PIT_H
#define MODULES_PIT_H

#define PIT_HERTZ     1193180
#define PIT_FREQUENCY 1000

#define PIT_DEVICE_TYPE 2202

struct pit_device
{

    struct modules_device base;
    unsigned short divisor;

};

extern void pit_device_init(struct pit_device *device);
extern void pit_init();

#endif

