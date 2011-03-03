#ifndef ARCH_KBD_H
#define ARCH_KBD_H

#define KBD_PORT_READ 0x60
#define KBD_BUFFER_SIZE 256

struct isr_registers;

extern void kbd_handler(struct isr_registers *registers);
extern void kbd_init();

#endif

