#ifndef KBD_H
#define KBD_H

#define KBD_BUFFER_SIZE 256

typedef struct kbd_device
{

    char buffer[KBD_BUFFER_SIZE];
    cbuffer_t cbuffer;
    uint16_t bufferSize;
    uint8_t toggleAlt;
    uint8_t toggleCtrl;
    uint8_t toggleShift;

} kbd_device_t;

extern uint16_t kbd_buffer_size();
extern char kbd_buffer_read(kbd_device_t *device);
extern void kbd_init();

#endif

