#ifndef MODULES_PS2_H
#define MODULES_PS2_H

#define KBD_PORT_READ 0x60

#define KBD_DEVICE_TYPE   0x3205
#define KBD_DRIVER_TYPE   0x0005
#define MOUSE_DEVICE_TYPE 0x4205
#define MOUSE_DRIVER_TYPE 0x0059

struct kbd_buffer
{

    char buffer[256];
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    unsigned int (*getc)(struct kbd_buffer *self, char *buffer);
    unsigned int (*putc)(struct kbd_buffer *self, char *buffer);

};

struct kbd_device
{

    struct modules_device base;
    unsigned int irq;

};

struct kbd_driver
{

    struct modules_driver base;
    struct kbd_buffer buffer;
    char *map;
    unsigned int escaped;
    unsigned int toggleAlt;
    unsigned int toggleCtrl;
    unsigned int toggleShift;

};

struct mouse_device
{

    struct modules_device base;
    unsigned int irq;

};

struct mouse_driver
{

    struct modules_driver base;
    unsigned char cycle;
    char status;
    char x;
    char y;

};

void kbd_device_init(struct kbd_device *device, unsigned int irq);
void kbd_driver_init(struct kbd_driver *driver);
void mouse_device_init(struct mouse_device *device, unsigned int irq);
void mouse_driver_init(struct mouse_driver *driver);

#endif

