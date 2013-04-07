#define PIT_COUNTER0                    0x40
#define PIT_COUNTER1                    0x41
#define PIT_COUNTER2                    0x42
#define PIT_COMMAND                     0x43

#define PIT_COMMAND_BINARY              0x00
#define PIT_COMMAND_BCD                 0x01

#define PIT_COMMAND_MODE0               0x00
#define PIT_COMMAND_MODE1               0x02
#define PIT_COMMAND_MODE2               0x04
#define PIT_COMMAND_MODE3               0x06
#define PIT_COMMAND_MODE4               0x08
#define PIT_COMMAND_MODE5               0x0A

#define PIT_COMMAND_LATCH               0x00
#define PIT_COMMAND_LOW                 0x10
#define PIT_COMMAND_HIGH                0x20
#define PIT_COMMAND_BOTH                0x30

#define PIT_COMMAND_COUNTER0            0x00
#define PIT_COMMAND_COUNTER1            0x40
#define PIT_COMMAND_COUNTER2            0x80
#define PIT_COMMAND_READBACK            0xC0

#define PIT_FREQUENCY                   1193182
#define PIT_HERTZ                       100

#define PIT_IRQ                         0x00

#define PIT_DEVICE_TYPE                 0x2002

struct pit_device
{

    struct base_device base;
    unsigned short divisor;
    unsigned short jiffies;

};

struct pit_driver
{

    struct base_driver base;
    struct timer_interface itimer;

};

void pit_init_device(struct pit_device *device, unsigned int irq);
void pit_init_driver(struct pit_driver *driver);
