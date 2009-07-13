#ifndef KBD_H
#define KBD_H

#define KBD_BUFFER_SIZE 256

extern uint16_t kbd_buffer_size();
extern char kbd_buffer_read();
extern void kbd_init();

#endif

