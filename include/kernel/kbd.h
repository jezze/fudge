#ifndef KBD_H
#define KBD_H

#define KBD_PORT_READ 0x60
#define KBD_BUFFER_SIZE 256

struct cbuffer;
struct isr_registers;
struct vfs_node;

typedef struct kbd_device
{

    char buffer[KBD_BUFFER_SIZE];
    struct cbuffer cbuffer;
    unsigned char toggleAlt;
    unsigned char toggleCtrl;
    unsigned char toggleShift;

} kbd_device_t;

extern kbd_device_t keyboard;

extern void kbd_handler(struct isr_registers *registers);
extern void kbd_init();
extern struct vfs_node *kbd_get_node();

#endif

