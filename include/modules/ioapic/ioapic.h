#ifndef MODULES_IOAPIC_H
#define MODULES_IOAPIC_H

#define IOAPIC_TABLE_SLOTS 16

#define IOAPIC_DRIVER_TYPE 0x9024

struct ioapic_routine
{

    struct modules_device *device;
    void (*callback)(struct modules_device *device);

};

struct ioapic_driver
{

    struct modules_driver base;
    unsigned int (*register_routine)(struct ioapic_driver *self, unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device));
    unsigned int (*unregister_routine)(struct ioapic_driver *self, unsigned int index, struct modules_device *device);

};

unsigned int ioapic_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device));
unsigned int ioapic_unregister_routine(unsigned int index, struct modules_device *device);
void ioapic_driver_init(struct ioapic_driver *driver);

#endif

