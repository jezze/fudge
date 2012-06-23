#ifndef MODULES_APIC_H
#define MODULES_APIC_H

#define APIC_TABLE_SLOTS 16

#define APIC_DRIVER_TYPE 0x9024

struct apic_routine
{

    struct modules_device *device;
    void (*callback)(struct modules_device *device);

};

struct apic_driver
{

    struct modules_driver base;
    unsigned int (*register_routine)(struct apic_driver *self, unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device));
    unsigned int (*unregister_routine)(struct apic_driver *self, unsigned int index, struct modules_device *device);

};

unsigned int apic_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device));
unsigned int apic_unregister_routine(unsigned int index, struct modules_device *device);
void apic_driver_init(struct apic_driver *driver);

#endif

