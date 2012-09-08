#ifndef MODULES_PIT_H
#define MODULES_PIT_H

#define PIT_HERTZ     1193180
#define PIT_FREQUENCY 100

#define PIT_IRQ 0x00

#define PIT_DEVICE_TYPE 0x2202
#define PIT_DRIVER_TYPE 0x2202

struct pit_device
{

    struct base_device base;
    unsigned int irq;
    unsigned short divisor;
    unsigned short jiffies;

};

struct pit_driver
{

    struct base_driver base;

};

void pit_device_init(struct pit_device *device, unsigned int irq);
void pit_driver_init(struct pit_driver *driver);

#endif

