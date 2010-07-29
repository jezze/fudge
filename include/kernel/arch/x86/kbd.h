#ifndef KBD_H
#define KBD_H

#define KBD_PORT_READ 0x60
#define KBD_BUFFER_SIZE 256

struct cbuffer;
struct isr_registers;
struct vfs_node;

struct kbd_device
{

    char buffer[KBD_BUFFER_SIZE];
    struct cbuffer cbuffer;
    unsigned char toggleAlt;
    unsigned char toggleCtrl;
    unsigned char toggleShift;

};

extern struct kbd_device keyboard;

extern void kbd_handler(struct isr_registers *registers);
extern void kbd_init();
extern void kbd_set_node(struct vfs_node *node);

#endif

