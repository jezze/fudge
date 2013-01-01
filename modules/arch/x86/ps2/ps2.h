#define PS2_DATA                        0x60
#define PS2_COMMAND                     0x64

#define PS2_IRQ_KEYBOARD                0x01
#define PS2_IRQ_MOUSE                   0x0C

#define PS2_DEVICE_TYPE                 0x3205
#define PS2_KBD_DRIVER_TYPE             0x0005
#define PS2_MOUSE_DRIVER_TYPE           0x0059

struct ps2_device
{

    struct base_device base;
    unsigned int irq;

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

void ps2_init_device(struct ps2_device *device, char *name, unsigned int irq);
void ps2_init_kbd_driver(struct ps2_kbd_driver *driver);
void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);
