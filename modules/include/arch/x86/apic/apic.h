#ifndef MODULES_APIC_H
#define MODULES_APIC_H

#define APIC_MASTER_COMMAND    0x20
#define APIC_MASTER_DATA       0x21
#define APIC_SLAVE_COMMAND     0xA0
#define APIC_SLAVE_DATA        0xA1

#define APIC_COMMAND_CONFIG    0x11
#define APIC_COMMAND_EOI       0x20
#define APIC_DATA_8086         0x01
#define APIC_DATA_MASTERVECTOR 0x20
#define APIC_DATA_SLAVEVECTOR  0x28

#define APIC_TABLE_SLOTS 16

struct apic_routine
{

    struct base_device *device;
    void (*callback)(struct base_device *device);

};

unsigned int apic_register_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device));
unsigned int apic_unregister_routine(unsigned int index, struct base_device *device);

#endif

