#ifndef IO_H
#define IO_H

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern void outb(uint16_t port, uint8_t data);

#endif

