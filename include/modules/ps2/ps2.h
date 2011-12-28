#ifndef MODULES_PS2_H
#define MODULES_PS2_H

#define KBD_PORT_READ 0x60

#define KBD_DRIVER_TYPE 5
#define MOUSE_DRIVER_TYPE 59

struct kbd_buffer
{

    char buffer[256];
    unsigned int size;
    unsigned int head;
    unsigned int tail;
    unsigned int (*getc)(struct kbd_buffer *self, char *buffer);
    unsigned int (*putc)(struct kbd_buffer *self, char *buffer);

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

struct mouse_driver
{

    struct modules_driver base;
    unsigned char cycle;
    char byte[3];
    char x;
    char y;

};

extern void kbd_driver_init(struct kbd_driver *driver);
extern void mouse_driver_init(struct mouse_driver *driver);

#endif

