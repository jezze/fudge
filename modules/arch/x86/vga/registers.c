#include <module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include "vga.h"
#include "registers.h"

unsigned int inar(unsigned char index)
{

    io_outb(VGA_REGISTER_ARINDEX, index);

    return io_inb(VGA_REGISTER_ARREAD);

}

unsigned int incrt1(unsigned char index)
{

    io_outb(VGA_REGISTER_CRTINDEX1, index);

    return io_inb(VGA_REGISTER_CRTDATA1);

}

unsigned int insr(unsigned char index)
{

    io_outb(VGA_REGISTER_SRINDEX, index);

    return io_inb(VGA_REGISTER_SRDATA);

}

void outar(unsigned char index, unsigned char value)
{

    io_outb(VGA_REGISTER_ARINDEX, index);
    io_outb(VGA_REGISTER_ARWRITE, value);

}

void outcrt1(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_CRTINDEX1, (value << 8) | index);

}

void outgr(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_GRINDEX, (value << 8) | index);

}

void outsr(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_SRINDEX, (value << 8) | index);

}

