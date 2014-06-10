#define PS2_KEYBOARD_IRQ                0x01
#define PS2_KEYBOARD_DEVICE_TYPE        0x0006
#define PS2_MOUSE_IRQ                   0x0C
#define PS2_MOUSE_DEVICE_TYPE           0x0007

struct ps2_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct base_device item[2]; unsigned int count;} devices;

};

void ps2_device_enable(struct base_device *device);
void ps2_device_enable_interrupt(struct base_device *device);
unsigned char ps2_bus_read_status(struct ps2_bus *bus);
unsigned char ps2_bus_read_data(struct ps2_bus *bus);
unsigned char ps2_bus_read_data_async(struct ps2_bus *bus);
void ps2_bus_write_command(struct ps2_bus *bus, unsigned char value);
void ps2_bus_write_data(struct ps2_bus *bus, unsigned char value);
void ps2_bus_reset(struct ps2_bus *bus);
void ps2_init_bus(struct ps2_bus *bus, unsigned short control, unsigned short data);
