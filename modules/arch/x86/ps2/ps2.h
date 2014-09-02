#define PS2_BUS_TYPE                    0x0003
#define PS2_KEYBOARD_DEVICE_TYPE        0x0001
#define PS2_MOUSE_DEVICE_TYPE           0x0002

unsigned char ps2_getdata(struct base_bus *bus);
void ps2_setcommand(struct base_bus *bus, unsigned char value);
void ps2_setdata(struct base_bus *bus, unsigned char value);
void ps2_enable(struct base_bus *bus, unsigned int id);
void ps2_enableinterrupt(struct base_bus *bus, unsigned int id);
void ps2_reset(struct base_bus *bus, unsigned int id);
void ps2_identify(struct base_bus *bus, unsigned int id);
void ps2_enablescanning(struct base_bus *bus, unsigned int id);
void ps2_disablescanning(struct base_bus *bus, unsigned int id);
void ps2_default(struct base_bus *bus, unsigned int id);
