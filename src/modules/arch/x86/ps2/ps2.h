#define PS2_BUS                         0x0003
#define PS2_KEYBOARD                    0x0001
#define PS2_MOUSE                       0x0002

unsigned int ps2_checkdata(unsigned int id);
unsigned char ps2_getdata(void);
unsigned char ps2_getstatus(void);
unsigned short ps2_getirq(unsigned int id);
void ps2_enable(unsigned int id);
void ps2_disable(unsigned int id);
void ps2_enableinterrupt(unsigned int id);
void ps2_disableinterrupt(unsigned int id);
void ps2_reset(unsigned int id);
unsigned char ps2_identify(unsigned int id);
void ps2_enablescanning(unsigned int id);
void ps2_disablescanning(unsigned int id);
void ps2_enabletranslation(unsigned int id);
void ps2_disabletranslation(unsigned int id);
void ps2_default(unsigned int id);
void ps2_rate(unsigned int id, unsigned char rate);
