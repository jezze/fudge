#define PS2_BUS_TYPE                    0x0003
#define PS2_KEYBOARD_DEVICE_TYPE        0x0001
#define PS2_MOUSE_DEVICE_TYPE           0x0002

unsigned char ps2_bus_read_status(struct base_bus *bus);
unsigned char ps2_bus_read_data(struct base_bus *bus);
unsigned char ps2_bus_read_data_async(struct base_bus *bus);
void ps2_bus_write_command(struct base_bus *bus, unsigned char value);
void ps2_bus_write_data(struct base_bus *bus, unsigned char value);
void ps2_bus_enable_device(struct base_bus *bus, unsigned int id);
void ps2_bus_enable_interrupt(struct base_bus *bus, unsigned int id);
void ps2_bus_reset(struct base_bus *bus, unsigned int id);
void ps2_bus_identify(struct base_bus *bus, unsigned int id);
void ps2_bus_enable_scanning(struct base_bus *bus, unsigned int id);
void ps2_bus_disable_scanning(struct base_bus *bus, unsigned int id);
