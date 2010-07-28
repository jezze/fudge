#ifndef IO_H
#define IO_H

extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern unsigned short ind(unsigned short port);
extern void outb(unsigned short port, unsigned char data);
extern void outw(unsigned short port, unsigned short data);
extern void outd(unsigned short port, unsigned int data);

#endif

