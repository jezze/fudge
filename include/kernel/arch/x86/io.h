#ifndef IO_H
#define IO_H

extern unsigned char io_inb(unsigned short port);
extern unsigned short io_inw(unsigned short port);
extern unsigned int io_ind(unsigned short port);
extern void io_outb(unsigned short port, unsigned char data);
extern void io_outw(unsigned short port, unsigned short data);
extern void io_outd(unsigned short port, unsigned int data);

#endif

