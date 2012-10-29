#ifndef MODULES_PS2_H
#define MODULES_PS2_H

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

struct ps2_kbd_buffer
{

    char buffer[256];
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    unsigned int (*read)(struct ps2_kbd_buffer *self, unsigned int count, char *buffer);
    unsigned int (*write)(struct ps2_kbd_buffer *self, unsigned int count, char *buffer);

};

struct ps2_kbd_driver
{

    struct base_driver base;
    struct ps2_kbd_buffer buffer;
    char *map;
    unsigned int escaped;
    unsigned int toggleAlt;
    unsigned int toggleCtrl;
    unsigned int toggleShift;

};

struct ps2_mouse_driver
{

    struct base_driver base;
    unsigned char cycle;
    char status;
    char x;
    char y;

};

void ps2_init_device(struct ps2_device *device, char *name, unsigned int irq);
void ps2_init_kbd_driver(struct ps2_kbd_driver *driver);
void ps2_init_mouse_driver(struct ps2_mouse_driver *driver);

#endif

