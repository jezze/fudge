#define APIC_ROUTINE_SLOTS              16

#define APIC_COMMAND0                   0x20
#define APIC_DATA0                      0x21
#define APIC_COMMAND1                   0xA0
#define APIC_DATA1                      0xA1

#define APIC_COMMAND_CONFIG             0x11
#define APIC_COMMAND_EOI                0x20
#define APIC_DATA_8086                  0x01
#define APIC_DATA_VECTOR0               0x20
#define APIC_DATA_VECTOR1               0x28

struct apic_general_registers
{

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

};

struct apic_interrupt_registers
{

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;

};

struct apic_registers
{

    unsigned int ds;
    struct apic_general_registers general;
    unsigned int index;
    unsigned int slave;
    struct apic_interrupt_registers interrupt;

};

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
void apic_interrupt(struct apic_registers *registers);
unsigned int apic_set_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device));
unsigned int apic_unset_routine(unsigned int index, struct base_device *device);
