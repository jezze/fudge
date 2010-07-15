#ifndef KBD_H
#define KBD_H

#define KBD_PORT_READ 0x60
#define KBD_BUFFER_SIZE 256

typedef struct kbd_device
{

    char buffer[KBD_BUFFER_SIZE];
    cbuffer_t cbuffer;
    unsigned char toggleAlt;
    unsigned char toggleCtrl;
    unsigned char toggleShift;

} kbd_device_t;

extern kbd_device_t keyboard;

extern vfs_node_t vfsKeyboard;

extern void kbd_handler(isr_registers_t *registers);
extern void kbd_init();

#endif

