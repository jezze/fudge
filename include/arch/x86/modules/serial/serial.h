#ifndef ARCH_SERIAL_H
#define ARCH_SERIAL_H

#define SERIAL_COM1 0x03F8
#define SERIAL_COM2 0x02F8
#define SERIAL_COM3 0x03E8
#define SERIAL_COM4 0x02E8

extern char serial_inb(unsigned short port);
extern void serial_outb(unsigned short port, char c);
extern unsigned int serial_read(unsigned short port, unsigned int count, char *buffer);
extern unsigned int serial_write(unsigned short port, unsigned int count, char *buffer);
extern void serial_init();

#endif

