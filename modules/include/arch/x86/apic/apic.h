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

void apic_routine00();
void apic_routine01();
void apic_routine02();
void apic_routine03();
void apic_routine04();
void apic_routine05();
void apic_routine06();
void apic_routine07();
void apic_routine08();
void apic_routine09();
void apic_routine0A();
void apic_routine0B();
void apic_routine0C();
void apic_routine0D();
void apic_routine0E();
void apic_routine0F();
unsigned int apic_register_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device));
unsigned int apic_unregister_routine(unsigned int index, struct base_device *device);

#endif

