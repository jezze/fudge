#define PS2_IRQ_KEYBOARD                0x01
#define PS2_IRQ_MOUSE                   0x0C
#define PS2_BUS_TYPE                    0x0006
#define PS2_DEVICE_TYPE                 0x0006

struct ps2_device
{

    struct base_device base;

};

struct ps2_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct ps2_device item[2]; unsigned int count;} devices;

};

struct ps2_keyboard_stream
{

    char buffer[512];
    unsigned int head;
    unsigned int tail;

};

struct ps2_keyboard_driver
{

    struct base_driver base;
    struct base_keyboard_interface ikeyboard;
    struct ps2_keyboard_stream stream;
    unsigned int escaped;
    unsigned int ctrl;
    unsigned int alt;
    unsigned int shift;

};

struct ps2_mouse_driver
{

    struct base_driver base;
    struct base_mouse_interface imouse;
    unsigned char cycle;
    char status;

};

unsigned char ps2_bus_read_status(struct ps2_bus *bus);
unsigned char ps2_bus_read_data(struct ps2_bus *bus);
unsigned char ps2_bus_read_data_async(struct ps2_bus *bus);
void ps2_bus_write_command(struct ps2_bus *bus, unsigned char value);
void ps2_bus_write_data(struct ps2_bus *bus, unsigned char value);
void ps2_bus_reset(struct ps2_bus *bus);
void ps2_init_bus(struct ps2_bus *bus, unsigned short control, unsigned short data);
void ps2_init_device(struct ps2_device *device, struct ps2_bus *bus, unsigned int irq);
void ps2_init_keyboard_driver(struct ps2_keyboard_driver *driver);
void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
