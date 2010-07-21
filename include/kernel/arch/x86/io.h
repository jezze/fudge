#ifndef IO_H
#define IO_H

extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

#endif

