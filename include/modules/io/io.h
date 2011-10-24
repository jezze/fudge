#ifndef MODULES_IO_H
#define MODULES_IO_H

#define IO_DEVICE_TYPE 1

struct io_device
{

    struct modules_device base;
    unsigned char (*inb)(unsigned short port);
    unsigned short (*inw)(unsigned short port);
    unsigned int (*ind)(unsigned short port);
    void (*outb)(unsigned short port, unsigned char data);
    void (*outw)(unsigned short port, unsigned short data);
    void (*outd)(unsigned short port, unsigned int data);

};

extern unsigned char io_inb(unsigned short port);
extern unsigned short io_inw(unsigned short port);
extern unsigned int io_ind(unsigned short port);
extern void io_outb(unsigned short port, unsigned char data);
extern void io_outw(unsigned short port, unsigned short data);
extern void io_outd(unsigned short port, unsigned int data);
extern void io_device_init(struct io_device *device);
extern void io_init();

#endif

