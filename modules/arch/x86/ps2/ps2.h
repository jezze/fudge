#define PS2_BUS_TYPE                    0x0003
#define PS2_KEYBOARD_DEVICE_TYPE        0x0001
#define PS2_MOUSE_DEVICE_TYPE           0x0002

unsigned char ps2_getdata();
unsigned short ps2_getirq(unsigned int id);
void ps2_setcommand(unsigned char value);
void ps2_setdata(unsigned char value);
void ps2_enable(unsigned int id);
void ps2_enableinterrupt(unsigned int id);
void ps2_reset(unsigned int id);
void ps2_identify(unsigned int id);
void ps2_enablescanning(unsigned int id);
void ps2_disablescanning(unsigned int id);
void ps2_default(unsigned int id);
