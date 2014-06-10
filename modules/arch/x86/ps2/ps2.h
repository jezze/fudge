#define PS2_KEYBOARD_DEVICE_TYPE        0x0006
#define PS2_MOUSE_DEVICE_TYPE           0x0007

struct ps2_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;

};

unsigned char ps2_bus_read_status(struct ps2_bus *bus);
unsigned char ps2_bus_read_data(struct ps2_bus *bus);
unsigned char ps2_bus_read_data_async(struct ps2_bus *bus);
void ps2_bus_write_command(struct ps2_bus *bus, unsigned char value);
void ps2_bus_write_data(struct ps2_bus *bus, unsigned char value);
void ps2_bus_reset(struct ps2_bus *bus);
void ps2_bus_enable_device(struct ps2_bus *bus, unsigned int type);
void ps2_bus_enable_interrupt(struct ps2_bus *bus, unsigned int type);
