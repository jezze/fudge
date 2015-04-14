#include <fudge.h>
#include <arch/x86/io/io.h>
#include "registers.h"

#define VGA_CR_COUNT                    24
#define VGA_AR_COUNT                    21
#define VGA_GR_COUNT                    9
#define VGA_SR_COUNT                    5
#define VGA_MR_COUNT                    1

#define VGA_CR_OFFSET                   0
#define VGA_AR_OFFSET                   24
#define VGA_GR_OFFSET                   45
#define VGA_SR_OFFSET                   54
#define VGA_MR_OFFSET                   59

static unsigned char text1[0x10000];
static unsigned char text2[0x10000];
static unsigned char font1[0x10000];
static unsigned char font2[0x10000];

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

void vga_save()
{

    io_inb(0x3DA);
    outar(0x20 | 0x10, 0x01);
    outsr(0x04, 0x06);
    outgr(0x05, 0x00);
    outgr(0x06, 0x05);
    outsr(0x02, 0x01);
    outgr(0x04, 0x00);
    memory_copy(text1, (void *)0xA0000, 0x10000);
    outsr(0x02, 0x02);
    outgr(0x04, 0x01);
    memory_copy(text2, (void *)0xA0000, 0x10000);
    outsr(0x02, 0x04);
    outgr(0x04, 0x02);
    memory_copy(font1, (void *)0xA0000, 0x10000);
    outsr(0x02, 0x08);
    outgr(0x04, 0x03);
    memory_copy(font2, (void *)0xA0000, 0x10000);

}

void vga_restore()
{

    io_inb(0x3DA);
    outar(0x20 | 0x10, 0x01);
    outsr(0x04, 0x06);
    outgr(0x05, 0x00);
    outgr(0x06, 0x05);
    outsr(0x02, 0x01);
    outgr(0x04, 0x00);
    memory_copy((void *)0xA0000, text1, 0x10000);
    outsr(0x02, 0x02);
    outgr(0x04, 0x01);
    memory_copy((void *)0xA0000, text2, 0x10000);
    outsr(0x02, 0x04);
    outgr(0x04, 0x02);
    memory_copy((void *)0xA0000, font1, 0x10000);
    outsr(0x02, 0x08);
    outgr(0x04, 0x03);
    memory_copy((void *)0xA0000, font2, 0x10000);

}

void vga_settext()
{

    io_outb(0x3C2, 0x67);
    outsr(0x00, 0x03);
    outsr(0x01, 0x00);
    outsr(0x02, 0x03);
    outsr(0x03, 0x00);
    outsr(0x04, 0x02);
    outcrt1(0x11, 0x0E);
    outcrt1(0x00, 0x5F);
    outcrt1(0x01, 0x4F);
    outcrt1(0x02, 0x50);
    outcrt1(0x03, 0x82);
    outcrt1(0x04, 0x55);
    outcrt1(0x05, 0x81);
    outcrt1(0x06, 0xBF);
    outcrt1(0x07, 0x1F);
    outcrt1(0x08, 0x00);
    outcrt1(0x09, 0x4F);
    outcrt1(0x10, 0x9C);
    outcrt1(0x11, 0x8E);
    outcrt1(0x12, 0x8F);
    outcrt1(0x13, 0x28);
    outcrt1(0x14, 0x1F);
    outcrt1(0x15, 0x96);
    outcrt1(0x16, 0xB9);
    outcrt1(0x17, 0xA3);
    outcrt1(0x18, 0xFF);
    outgr(0x00, 0x00);
    outgr(0x01, 0x00);
    outgr(0x02, 0x00);
    outgr(0x03, 0x00);
    outgr(0x04, 0x00);
    outgr(0x05, 0x10);
    outgr(0x06, 0x0E);
    outgr(0x07, 0x00);
    outgr(0x08, 0xFF);
    io_inb(0x3DA);
    outar(0x00, 0x00);
    outar(0x01, 0x01);
    outar(0x02, 0x02);
    outar(0x03, 0x03);
    outar(0x04, 0x04);
    outar(0x05, 0x05);
    outar(0x06, 0x06);
    outar(0x07, 0x07);
    outar(0x08, 0x38);
    outar(0x09, 0x39);
    outar(0x0A, 0x3A);
    outar(0x0B, 0x3B);
    outar(0x0C, 0x3C);
    outar(0x0D, 0x3D);
    outar(0x0E, 0x3E);
    outar(0x0F, 0x3F);
    outar(0x20 | 0x10, 0x0C);
    outar(0x20 | 0x11, 0x00);
    outar(0x20 | 0x12, 0x0F);
    outar(0x20 | 0x13, 0x08);
    outar(0x20 | 0x14, 0x00);

}

void vga_setgraphics()
{

    io_outb(0x3C2, 0x63);
    outsr(0x00, 0x03);
    outsr(0x01, 0x01);
    outsr(0x02, 0x0F);
    outsr(0x03, 0x00);
    outsr(0x04, 0x0E);
    outcrt1(0x11, 0x0E);
    outcrt1(0x00, 0x5F);
    outcrt1(0x01, 0x4F);
    outcrt1(0x02, 0x50);
    outcrt1(0x03, 0x82);
    outcrt1(0x04, 0x54);
    outcrt1(0x05, 0x80);
    outcrt1(0x06, 0xBF);
    outcrt1(0x07, 0x1F);
    outcrt1(0x08, 0x00);
    outcrt1(0x09, 0x41);
    outcrt1(0x10, 0x9C);
    outcrt1(0x11, 0x8E);
    outcrt1(0x12, 0x8F);
    outcrt1(0x13, 0x28);
    outcrt1(0x14, 0x40);
    outcrt1(0x15, 0x96);
    outcrt1(0x16, 0xB9);
    outcrt1(0x17, 0xA3);
    outcrt1(0x18, 0xFF);
    outgr(0x00, 0x00);
    outgr(0x01, 0x00);
    outgr(0x02, 0x00);
    outgr(0x03, 0x00);
    outgr(0x04, 0x00);
    outgr(0x05, 0x40);
    outgr(0x06, 0x05);
    outgr(0x07, 0x0F);
    outgr(0x08, 0xFF);
    io_inb(0x3DA);
    outar(0x00, 0x00);
    outar(0x01, 0x01);
    outar(0x02, 0x02);
    outar(0x03, 0x03);
    outar(0x04, 0x04);
    outar(0x05, 0x05);
    outar(0x06, 0x06);
    outar(0x07, 0x07);
    outar(0x08, 0x08);
    outar(0x09, 0x09);
    outar(0x0A, 0x0A);
    outar(0x0B, 0x0B);
    outar(0x0C, 0x0C);
    outar(0x0D, 0x0D);
    outar(0x0E, 0x0E);
    outar(0x0F, 0x0F);
    outar(0x20 | 0x10, 0x41);
    outar(0x20 | 0x11, 0x00);
    outar(0x20 | 0x12, 0x0F);
    outar(0x20 | 0x13, 0x00);
    outar(0x20 | 0x14, 0x00);

}

#define V_DBLSCAN 0x20
#define V_CLKDIV2 0x00
#define BIT_PLANE 3
#define PHSYNC                          0x0001
#define NHSYNC                          0x0002
#define PVSYNC                          0x0004
#define NVSYNC                          0x0008

void vga_initregisters(unsigned char *registers, unsigned int flags, unsigned int vdisplay, unsigned int vscan, unsigned int clockindex, unsigned int depth,
    unsigned int htotal, unsigned int hdisplay, unsigned int hsyncstart, unsigned int hsyncend, unsigned int hskew,
    unsigned int vtotal, unsigned int vsyncstart, unsigned int vsyncend, unsigned int vblankstart, unsigned int vblankend,
    unsigned int linewidth)
{

    unsigned int i;
    unsigned char misc;

    if ((flags & (PHSYNC | NHSYNC)) && (flags & (PVSYNC | NVSYNC)))
    {

        misc = 0x23;

        if (flags & NHSYNC)
            misc |= 0x40;

        if (flags & NVSYNC)
            misc |= 0x80;

    }

    else
    {

        unsigned int v = vdisplay;

        if (flags & V_DBLSCAN)
            v *= 2;

        if (vscan > 1)
            v *= vscan;

        if (v < 400)
            misc = 0xA3;
        else if (v < 480)
            misc = 0x63;
        else if (v < 768)
            misc = 0xE3;
        else
            misc = 0x23;

    }

    misc |= (clockindex & 0x03) << 2;

    io_outb(0x3C2, misc);

    if (depth == 4)
        registers[VGA_SR_OFFSET] = 0x02;
    else
        registers[VGA_SR_OFFSET] = 0x00;

    if (flags & V_CLKDIV2)
        registers[VGA_SR_OFFSET + 1] = 0x09;
    else
        registers[VGA_SR_OFFSET + 1] = 0x01;

    if (depth == 1)
        registers[VGA_SR_OFFSET + 2] = 1 << BIT_PLANE;
    else
        registers[VGA_SR_OFFSET + 2] = 0x0F;

    registers[VGA_SR_OFFSET + 3] = 0x00;

    if (depth < 8)
        registers[VGA_SR_OFFSET + 4] = 0x06;
    else
        registers[VGA_SR_OFFSET + 4] = 0x0E;

    registers[VGA_CR_OFFSET + 0] = (htotal >> 3) - 5;
    registers[VGA_CR_OFFSET + 1] = (hdisplay >> 3) - 1;
    registers[VGA_CR_OFFSET + 2] = (hsyncstart >> 3) - 1;
    registers[VGA_CR_OFFSET + 3] = ((hsyncend >> 3) & 0x1F) | 0x80;

    i = (((hskew << 2) + 0x10) & ~0x1F);

    if (i < 0x80)
        registers[VGA_CR_OFFSET + 3] |= i;

    registers[VGA_CR_OFFSET + 4] = (hsyncstart >> 3);
    registers[VGA_CR_OFFSET + 5] = ((((hsyncend >> 3) - 1) & 0x20) << 2) | ((hsyncend >> 3) & 0x1F);
    registers[VGA_CR_OFFSET + 6] = (vtotal - 2) & 0xFF;
    registers[VGA_CR_OFFSET + 7] = (((vtotal - 2) & 0x100) >> 8) | (((vdisplay - 1) & 0x100) >> 7) | ((vsyncstart & 0x100) >> 6) | (((vblankstart - 1) & 0x100) >> 5) | 0x10 | (((vtotal - 2) & 0x200) >> 4) | (((vdisplay - 1) & 0x200) >> 3) | ((vsyncstart & 0x200) >> 2);
    registers[VGA_CR_OFFSET + 8] = 0x00;
    registers[VGA_CR_OFFSET + 9] = (((vblankstart - 1) & 0x200) >> 4) | 0x40;

    if (flags & V_DBLSCAN)
        registers[VGA_CR_OFFSET + 9] |= 0x80;

    if (vscan >= 32)
        registers[VGA_CR_OFFSET + 9] |= vscan - 1;

    registers[VGA_CR_OFFSET + 16] = vsyncstart & 0xFF;
    registers[VGA_CR_OFFSET + 17] = (vsyncend & 0x0F) | 0x20;
    registers[VGA_CR_OFFSET + 18] = (vdisplay - 1) & 0xFF;
    registers[VGA_CR_OFFSET + 19] = linewidth >> 4;
    registers[VGA_CR_OFFSET + 20] = 0x00;
    registers[VGA_CR_OFFSET + 21] = (vblankstart - 1) & 0xFF;
    registers[VGA_CR_OFFSET + 22] = (vblankend - 1) & 0xFF;

    if (depth < 8)
        registers[VGA_CR_OFFSET + 23] = 0xE3;
    else
        registers[VGA_CR_OFFSET + 23] = 0xC3;

    registers[VGA_CR_OFFSET + 24] = 0xFF;

    registers[VGA_GR_OFFSET + 0] = 0x00;
    registers[VGA_GR_OFFSET + 1] = 0x00;
    registers[VGA_GR_OFFSET + 2] = 0x00;
    registers[VGA_GR_OFFSET + 3] = 0x00;

    if (depth == 1)
    {

        registers[VGA_GR_OFFSET + 4] = BIT_PLANE;
        registers[VGA_GR_OFFSET + 5] = 0x00;

    }

    else
    {

        registers[VGA_GR_OFFSET + 4] = 0x00;

        if (depth == 4)
            registers[VGA_GR_OFFSET + 5] = 0x02;
        else
            registers[VGA_GR_OFFSET + 5] = 0x40;

    }

    registers[VGA_GR_OFFSET + 6] = 0x05;
    registers[VGA_GR_OFFSET + 7] = 0x0F;
    registers[VGA_GR_OFFSET + 8] = 0xFF;

    if (depth == 1)
    {

    }

    else
    {

        registers[VGA_AR_OFFSET + 0] = 0x00;
        registers[VGA_AR_OFFSET + 1] = 0x01;
        registers[VGA_AR_OFFSET + 2] = 0x02;
        registers[VGA_AR_OFFSET + 3] = 0x03;
        registers[VGA_AR_OFFSET + 4] = 0x04;
        registers[VGA_AR_OFFSET + 5] = 0x05;
        registers[VGA_AR_OFFSET + 6] = 0x06;
        registers[VGA_AR_OFFSET + 7] = 0x07;
        registers[VGA_AR_OFFSET + 8] = 0x08;
        registers[VGA_AR_OFFSET + 9] = 0x09;
        registers[VGA_AR_OFFSET + 10] = 0x0A;
        registers[VGA_AR_OFFSET + 11] = 0x0B;
        registers[VGA_AR_OFFSET + 12] = 0x0C;
        registers[VGA_AR_OFFSET + 13] = 0x0D;
        registers[VGA_AR_OFFSET + 14] = 0x0E;
        registers[VGA_AR_OFFSET + 15] = 0x0F;

        if (depth == 4)
            registers[VGA_AR_OFFSET + 16] = 0x81;
        else
            registers[VGA_AR_OFFSET + 16] = 0x41;

    }

    registers[VGA_AR_OFFSET + 18] = 0x0F;
    registers[VGA_AR_OFFSET + 19] = 0x00;
    registers[VGA_AR_OFFSET + 20] = 0x00;

}

