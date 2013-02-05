#define PS2_BUS_DATA                    0x0060
#define PS2_BUS_CONTROL                 0x0064

#define PS2_IRQ_KEYBOARD                0x01
#define PS2_IRQ_MOUSE                   0x0C

#define PS2_BUS_TYPE                    0x0006
#define PS2_DEVICE_TYPE                 0x0006

struct ps2_bus;

struct ps2_device
{

    struct base_device base;
    struct ps2_bus *bus;

};

struct ps2_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct ps2_device item[2]; unsigned int count;} devices;

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

unsigned char ps2_bus_read_status(struct ps2_bus *bus);
unsigned char ps2_bus_read_data(struct ps2_bus *bus);
unsigned char ps2_bus_read_data_async(struct ps2_bus *bus);
void ps2_bus_write_command(struct ps2_bus *bus, unsigned char value);
void ps2_bus_write_data(struct ps2_bus *bus, unsigned char value);
void ps2_bus_reset(struct ps2_bus *bus);
void ps2_init_bus(struct ps2_bus *bus);
void ps2_init_device(struct ps2_device *device, struct ps2_bus *bus, char *name, unsigned int irq);
void ps2_init_kbd_driver(struct ps2_kbd_driver *driver);
void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
