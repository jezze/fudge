#include <fudge/module.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include "vga.h"

#define SZ(x) (sizeof(x)/sizeof(x[0]))

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
    unsigned char *address = (void *)VGA_ADDRESS;

    for (i = 0; i < 320 * 240; i++)
        *(address + i) = 0x00;

}

static void plot(unsigned int offset, unsigned char color)
{

    unsigned char *address = (void *)(VGA_ADDRESS + offset);

    *(address) = color;

}

static void mode(int width, int height, int chain4)
{

    unsigned char *w, *h;
    unsigned char val;
    int a;

    switch (width)
    {

    case 256:
        w = width_256;
        val = VGA_RESOLUTION_COM + VGA_RESOLUTION_W256;

        break;

    case 320:
        w = width_320;
        val = VGA_RESOLUTION_COM + VGA_RESOLUTION_W320;

        break;

    case 360:
        w = width_360;
        val = VGA_RESOLUTION_COM + VGA_RESOLUTION_W360;

        break;

    case 376:
        w = width_376;
        val = VGA_RESOLUTION_COM + VGA_RESOLUTION_W376;

        break;

    case 400:
        w = width_400;
        val = VGA_RESOLUTION_COM + VGA_RESOLUTION_W400;

        break;

    default:
        return;

    }

    switch (height)
    {

    case 200:
        h = height_200;
        val |= VGA_RESOLUTION_H200;

        break;

    case 224:
        h = height_224;
        val |= VGA_RESOLUTION_H224;

        break;

    case 240:
        h = height_240;
        val |= VGA_RESOLUTION_H240;

        break;

    case 256:
        h = height_256;
        val |= VGA_RESOLUTION_H256;

        break;

    case 270:
        h = height_270;
        val |= VGA_RESOLUTION_H270;

        break;

    case 300:
        h = height_300;
        val |= VGA_RESOLUTION_H300;

        break;

    case 360:
        h = height_360;
        val |= VGA_RESOLUTION_H360;

        break;

    case 400:
        h = height_400;
        val |= VGA_RESOLUTION_H400;

        break;

    case 480:
        h = height_480;
        val |= VGA_RESOLUTION_H480;

        break;

    case 564:
        h = height_564;
        val |= VGA_RESOLUTION_H564;

        break;

    case 600:
        h = height_600;
        val |= VGA_RESOLUTION_H600;

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

static void enable(struct video_interface *self)
{

    mode(320, 200, 1);
    clear();
    plot(0x00, 0x11);
    plot(160 * 100, 0x11);

}

static unsigned int read_framebuffer(struct vga_driver *driver, unsigned int offset, unsigned int count, void *buffer)
{

    volatile char *address = (volatile char *)VGA_FB_ADDRESS;
    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        memory_read(in + j, 1, (void *)(address + i * 2), 1, 0);

    }

    return j;

}

static unsigned int write_framebuffer(struct vga_driver *driver, unsigned int offset, unsigned int count, void *buffer)
{

    volatile char *address = (volatile char *)VGA_FB_ADDRESS;
    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        memory_write((void *)(address + i * 2 + 0), 1, in + j, 1, 0);
        memory_write((void *)(address + i * 2 + 1), 1, &driver->cursor.color, 1, 0);

    }

    return j;

}

static void scroll(struct vga_driver *driver)
{

    unsigned int i;
    char buffer[2000];

    read_framebuffer(driver, 80, 2000 - 80, buffer);

    for (i = 2000 - 80; i < 2000; i++)
        buffer[i] = ' ';

    write_framebuffer(driver, 0, 2000, buffer);

    driver->cursor.offset -= 80;

}

static void putc(struct vga_driver *driver, char c)
{

    if (c == '\b')
        driver->cursor.offset--;

    if (c == '\t')
        driver->cursor.offset = (driver->cursor.offset + 8) & ~(8 - 1);

    if (c == '\r')
        driver->cursor.offset -= (driver->cursor.offset % 80);

    if (c == '\n')
        driver->cursor.offset += 80 - (driver->cursor.offset % 80);

    if (c >= ' ')
    {

        write_framebuffer(driver, driver->cursor.offset, 1, &c);
        driver->cursor.offset++;

    }

    if (driver->cursor.offset >= 80 * 25)
        scroll(driver);

    io_outb(0x3D4, 14);
    io_outb(0x3D5, driver->cursor.offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, driver->cursor.offset);

}

static unsigned int read_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, (void *)VGA_FB_ADDRESS, 4000, offset);

}

static unsigned int write_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)self->driver;
    unsigned int i;

    for (i = 0; i < count; i++)
        putc(driver, ((char *)buffer)[i]);

    return count;

}

static void start(struct base_driver *self)
{

    struct vga_driver *driver = (struct vga_driver *)self;
    unsigned int i;

    driver->cursor.color = (VGA_FB_COLOR_BLACK << 4) | (VGA_FB_COLOR_WHITE & 0x0F);

    for (i = 0; i < 2000; i++)
        write_framebuffer(driver, i, 1, " ");

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));
    base_init_driver(&driver->base, "vga", start, 0, 0);
    video_init_interface(&driver->interface, &driver->base, enable, read_data, write_data);

    driver->interface.xres = 80;
    driver->interface.yres = 25;
    driver->interface.bpp = 4;

}

