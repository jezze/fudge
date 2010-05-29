#ifndef KBD_H
#define KBD_H

#define KBD_BUFFER_SIZE 256

typedef struct kbd_device
{

    char buffer[KBD_BUFFER_SIZE];
    cbuffer_t cbuffer;
    uint8_t toggleAlt;
    uint8_t toggleCtrl;
    uint8_t toggleShift;

} kbd_device_t;

extern kbd_device_t keyboard;
extern void kbd_init();

#endif

