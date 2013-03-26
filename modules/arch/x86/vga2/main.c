#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include <video/video.h>
#include <arch/x86/io/io.h>

#define SZ(x) (sizeof(x)/sizeof(x[0]))

#define R_COM 0x63

#define R_W256 0x00
#define R_W320 0x00
#define R_W360 0x04
#define R_W376 0x04
#define R_W400 0x04

#define R_H200 0x00
#define R_H224 0x80
#define R_H240 0x80
#define R_H256 0x80
#define R_H270 0x80
#define R_H300 0x80
#define R_H360 0x00
#define R_H400 0x00
#define R_H480 0x80
#define R_H564 0x80
#define R_H600 0x80

static unsigned char hor_regs[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x13};
static unsigned char width_256[] = {0x5F, 0x3F, 0x40, 0x82, 0x4A, 0x9A, 0x20};
static unsigned char width_320[] = {0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x28};
static unsigned char width_360[] = {0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0x2D};
static unsigned char width_376[] = {0x6E, 0x5D, 0x5E, 0x91, 0x62, 0x8F, 0x2F};
static unsigned char width_400[] = {0x70, 0x63, 0x64, 0x92, 0x65, 0x82, 0x32};
static unsigned char ver_regs[] = {0x06, 0x07, 0x09, 0x10, 0x11, 0x12, 0x15, 0x16};
static unsigned char height_200[] = {0xBF, 0x1F, 0x41, 0x9C, 0x8E, 0x8F, 0x96, 0xB9};
static unsigned char height_224[] = {0x0B, 0x3E, 0x41, 0xDA, 0x9C, 0xBF, 0xC7, 0x04};
static unsigned char height_240[] = {0x0D, 0x3E, 0x41, 0xEA, 0xAC, 0xDF, 0xE7, 0x06};
static unsigned char height_256[] = {0x23, 0xB2, 0x61, 0x0A, 0xAC, 0xFF, 0x07, 0x1A};
static unsigned char height_270[] = {0x30, 0xF0, 0x61, 0x20, 0xA9, 0x1B, 0x1F, 0x2F};
static unsigned char height_300[] = {0x70, 0xF0, 0x61, 0x5B, 0x8C, 0x57, 0x58, 0x70};
static unsigned char height_360[] = {0xBF, 0x1F, 0x40, 0x88, 0x85, 0x67, 0x6D, 0xBA};
static unsigned char height_400[] = {0xBF, 0x1F, 0x40, 0x9C, 0x8E, 0x8F, 0x96, 0xB9};
static unsigned char height_480[] = {0x0D, 0x3E, 0x40, 0xEA, 0xAC, 0xDF, 0xE7, 0x06};
static unsigned char height_564[] = {0x62, 0xF0, 0x60, 0x37, 0x89, 0x33, 0x3C, 0x5C};
static unsigned char height_600[] = {0x70, 0xF0, 0x60, 0x5B, 0x8C, 0x57, 0x58, 0x70};

static void clear()
{

    unsigned int i;
    unsigned char *address = (void *)0xA0000;

    for (i = 0; i < 320 * 240; i++)
        *(address + i) = 0x00;

}

static void plot(unsigned int offset, unsigned char color)
{

    unsigned char *address = (void *)(0xA0000 + offset);

    *(address) = color;

}

static void start(int width, int height, int chain4)
{

    unsigned char *w, *h;
    unsigned char val;
    int a;

    switch (width)
    {

    case 256:
        w = width_256;
        val = R_COM + R_W256;

        break;

    case 320:
        w = width_320;
        val = R_COM + R_W320;

        break;

    case 360:
        w = width_360;
        val = R_COM + R_W360;

        break;

    case 376:
        w = width_376;
        val = R_COM + R_W376;

        break;

    case 400:
        w = width_400;
        val = R_COM + R_W400;

        break;

    default:
        return;

    }

    switch (height)
    {

    case 200:
        h = height_200;
        val |= R_H200;

        break;

    case 224:
        h = height_224;
        val |= R_H224;

        break;

    case 240:
        h = height_240;
        val |= R_H240;

        break;

    case 256:
        h = height_256;
        val |= R_H256;

        break;

    case 270:
        h = height_270;
        val |= R_H270;

        break;

    case 300:
        h = height_300;
        val |= R_H300;

        break;

    case 360:
        h = height_360;
        val |= R_H360;

        break;

    case 400:
        h = height_400;
        val |= R_H400;

        break;

    case 480:
        h = height_480;
        val |= R_H480;

        break;

    case 564:
        h = height_564;
        val |= R_H564;

        break;

    case 600:
        h = height_600;
        val |= R_H600;

        break;

    default:
        return;

    }

    if (chain4 && (unsigned int)width * (unsigned int)height > 65536)
        return;

    io_outb(0x3C2, val);
    io_outw(0x3D4, 0x0E11);

    for (a = 0; a < SZ(hor_regs); ++a)
        io_outw(0x3D4, (unsigned short)((w[a] << 8) + hor_regs[a]));

    for (a = 0; a < SZ(ver_regs); ++a)
        io_outw(0x3D4, (unsigned short)((h[a] << 8) + ver_regs[a]));

    io_outw(0x3D4, 0x0008);

    if (chain4)
    {
    
        io_outw(0x3D4, 0x4014);
        io_outw(0x3D4, 0xA317);
        io_outw(0x3C4, 0x0E04);

    }

    else
    {

        io_outw(0x3D4, 0x0014);
        io_outw(0x3D4, 0xE317);
        io_outw(0x3C4, 0x0604);

    }

    io_outw(0x3C4, 0x0101);
    io_outw(0x3C4, 0x0F02);
    io_outw(0x3CE, 0x4005);
    io_outw(0x3CE, 0x0506);

    io_inb(0x3DA);
    io_outb(0x3C0, 0x30);
    io_outb(0x3C0, 0x41);
    io_outb(0x3C0, 0x33);
    io_outb(0x3C0, 0x00);

    for (a = 0; a < 16; a++)
    {

        io_outb(0x3C0, (unsigned char)a);
        io_outb(0x3C0, (unsigned char)a);

    }

    io_outb(0x3C0, 0x20);

}

void init()
{

    start(320, 200, 1);
    clear();
    plot(0x00, 0x11);
    plot(160 * 100, 0x11);

}

void destroy()
{

}
