#define PS2_KEYBOARD_DEVICE_TYPE        0x0006
#define PS2_MOUSE_DEVICE_TYPE           0x0007

unsigned char ps2_bus_read_status(struct base_bus *bus);
unsigned char ps2_bus_read_data(struct base_bus *bus);
unsigned char ps2_bus_read_data_async(struct base_bus *bus);
void ps2_bus_write_command(struct base_bus *bus, unsigned char value);
void ps2_bus_write_data(struct base_bus *bus, unsigned char value);
void ps2_bus_reset(struct base_bus *bus);
void ps2_bus_enable_device(struct base_bus *bus, unsigned int type);
void ps2_bus_enable_interrupt(struct base_bus *bus, unsigned int type);
