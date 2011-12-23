#ifndef MODULES_PS2_H
#define MODULES_PS2_H

#define KBD_PORT_READ 0x60

#define KBD_DEVICE_TYPE 5
#define MOUSE_DEVICE_TYPE 59

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
    unsigned char cycle;
    char byte[3];
    char x;
    char y;

};

extern void kbd_device_init(struct kbd_device *device);
extern void mouse_device_init(struct mouse_device *device);

#endif

