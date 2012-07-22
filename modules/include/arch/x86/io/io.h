#ifndef MODULES_IO_H
#define MODULES_IO_H

#define IO_DRIVER_TYPE 0x2812

struct io_driver
{

    struct modules_driver base;

};

volatile unsigned char io_inb(unsigned short port);
volatile unsigned short io_inw(unsigned short port);
volatile unsigned int io_ind(unsigned short port);
void io_outb(unsigned short port, unsigned char data);
void io_outw(unsigned short port, unsigned short data);
void io_outd(unsigned short port, unsigned int data);
void io_driver_init(struct io_driver *driver);

#endif

