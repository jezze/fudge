#define PS2_DATA                        0x60
#define PS2_COMMAND                     0x64

#define PS2_IRQ_KEYBOARD                0x01
#define PS2_IRQ_MOUSE                   0x0C

#define PS2_BUS_TYPE                    0x0006
#define PS2_DEVICE_TYPE                 0x0006

struct ps2_bus;

struct ps2_device
{

    struct base_device base;
    struct ps2_bus *bus;
    unsigned int irq;

};

struct ps2_bus
{

    struct base_bus base;
    struct ps2_device devices[2];
    unsigned int devicesCount;
    unsigned char (*read_status)();
    void (*wait_read)();
    void (*wait_write)();
    unsigned char (*read_data)();
    void (*write_command)(unsigned char value);
    void (*write_data)(unsigned char value);

};

struct ps2_kbd_driver
{

    struct base_driver base;
    struct kbd_interface interface;
    struct circular_stream stream;
    unsigned int escaped;

};

struct ps2_mouse_driver
{

    struct base_driver base;
    struct mouse_interface interface;
    unsigned char cycle;
    char status;

};

void ps2_init_bus(struct ps2_bus *bus);
void ps2_init_device(struct ps2_device *device, struct ps2_bus *bus, char *name, unsigned int irq);
void ps2_init_kbd_driver(struct ps2_kbd_driver *driver);
void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
