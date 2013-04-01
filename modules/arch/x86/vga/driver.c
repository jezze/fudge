#include <fudge/module.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include "vga.h"

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

static void clear(struct video_interface *interface)
{

    unsigned int i;
    unsigned char *address = (void *)VGA_ADDRESS;

    for (i = 0; i < interface->xres * interface->yres; i++)
        *(address + i) = 0x00;

}

static void mode(struct video_interface *interface, int chain4)
{

    unsigned char *w, *h;
    unsigned char settings;
    int a;

    if (chain4 && interface->xres * interface->yres > 65536)
        return;

    switch (interface->xres)
    {

        case 256:
            w = width_256;
            settings = VGA_RESOLUTION_COM + VGA_RESOLUTION_W256;

            break;

        case 320:
            w = width_320;
            settings = VGA_RESOLUTION_COM + VGA_RESOLUTION_W320;

            break;

        case 360:
            w = width_360;
            settings = VGA_RESOLUTION_COM + VGA_RESOLUTION_W360;

            break;

        case 376:
            w = width_376;
            settings = VGA_RESOLUTION_COM + VGA_RESOLUTION_W376;

            break;

        case 400:
            w = width_400;
            settings = VGA_RESOLUTION_COM + VGA_RESOLUTION_W400;

            break;

        default:
            return;

    }

    switch (interface->yres)
    {

        case 200:
            h = height_200;
            settings |= VGA_RESOLUTION_H200;

            break;

        case 224:
            h = height_224;
            settings |= VGA_RESOLUTION_H224;

            break;

        case 240:
            h = height_240;
            settings |= VGA_RESOLUTION_H240;

            break;

        case 256:
            h = height_256;
            settings |= VGA_RESOLUTION_H256;

            break;

        case 270:
            h = height_270;
            settings |= VGA_RESOLUTION_H270;

            break;

        case 300:
            h = height_300;
            settings |= VGA_RESOLUTION_H300;

            break;

        case 360:
            h = height_360;
            settings |= VGA_RESOLUTION_H360;

            break;

        case 400:
            h = height_400;
            settings |= VGA_RESOLUTION_H400;

            break;

        case 480:
            h = height_480;
            settings |= VGA_RESOLUTION_H480;

            break;

        case 564:
            h = height_564;
            settings |= VGA_RESOLUTION_H564;

            break;

        case 600:
            h = height_600;
            settings |= VGA_RESOLUTION_H600;

            break;

        default:
            return;

    }

    io_outb(VGA_REG_MISC_CTRL, settings);

    for (a = 0; a < 7; a++)
        io_outw(VGA_REG_CR_COLOR_INDEX, (unsigned short)((w[a] << 8) + hor_regs[a]));

    for (a = 0; a < 8; a++)
        io_outw(VGA_REG_CR_COLOR_INDEX, (unsigned short)((h[a] << 8) + ver_regs[a]));

    if (chain4)
    {
    
        io_outw(VGA_REG_CR_COLOR_INDEX, 0x4014);
        io_outw(VGA_REG_CR_COLOR_INDEX, 0xA317);
        io_outw(VGA_REG_SR_INDEX, 0x0E04);

    }

    else
    {

        io_outw(VGA_REG_CR_COLOR_INDEX, 0x0014);
        io_outw(VGA_REG_CR_COLOR_INDEX, 0xE317);
        io_outw(VGA_REG_SR_INDEX, 0x0604);

    }

    io_outw(VGA_REG_SR_INDEX, 0x0101); /* ASYNC RESET (1 << 0) */
    io_outw(VGA_REG_SR_INDEX, 0x0F02); /* HALFCLOCK (1 << 3) */
    io_outw(VGA_REG_GR_INDEX, 0x4005); /* 256 COLOR MODE = (1 << 6) */
    io_outw(VGA_REG_GR_INDEX, 0x0506); /* GRAPHICS MODE = (1 << 0), USE 0xA0000 MMAP (1 << 2) */
    io_inb(VGA_REG_FC_COLOR_CTRL);
    io_outb(VGA_REG_AR_INDEX, 0x30); /* DISPLAY ENABLE, SET AR REG 0x10 CONTROLLER MODE */
    io_outb(VGA_REG_AR_INDEX, 0x41); /* GRAPHICS MODE = (1 << 0), PIXEL DOUBLE CLOCK = (1 << 6)  */
    io_outb(VGA_REG_AR_INDEX, 0x33); /* DISPLAY ENABLE, SET AR REG 0x13 */
    io_outb(VGA_REG_AR_INDEX, 0x00); /* NO PANNING FOR PIXELS */

    for (a = 0; a < 16; a++)
    {

        io_outb(VGA_REG_AR_INDEX, a);
        io_outb(VGA_REG_AR_INDEX, a);

    }

    io_outb(VGA_REG_AR_INDEX, 0x20);

}

static void enable(struct video_interface *self)
{

    mode(self, 1);
    clear(self);

}

static unsigned int read_terminal_data(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_terminal_data(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)self->driver;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

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

            ((char *)VGA_FB_ADDRESS)[driver->cursor.offset * 2] = c;
            ((char *)VGA_FB_ADDRESS)[driver->cursor.offset * 2 + 1] = driver->cursor.color;

            driver->cursor.offset++;

        }

        if (driver->cursor.offset >= VGA_FB_SIZE)
        {

            unsigned int a;

            memory_copy((void *)VGA_FB_ADDRESS, (void *)(VGA_FB_ADDRESS + 80 * 2), 80 * 24 * 2);

            for (a = 80 * 24 * 2; a < 80 * 25 * 2; a += 2)
            {

                ((char *)VGA_FB_ADDRESS)[a] = ' ';
                ((char *)VGA_FB_ADDRESS)[a + 1] = driver->cursor.color;

            }

            driver->cursor.offset -= 80;

        }

        io_outb(VGA_REG_CR_COLOR_INDEX, 0x0E);
        io_outb(VGA_REG_CR_COLOR_DATA, driver->cursor.offset >> 8);
        io_outb(VGA_REG_CR_COLOR_INDEX, 0x0F);
        io_outb(VGA_REG_CR_COLOR_DATA, driver->cursor.offset);

    }

    return count;

}

static unsigned int read_video_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, (void *)VGA_ADDRESS, self->xres * self->yres * (self->bpp / 8), offset);

}

static unsigned int write_video_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_write((void *)VGA_ADDRESS, self->xres * self->yres * (self->bpp / 8), buffer, count, offset);

}

static unsigned int read_video_colormap(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_video_colormap(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    for (i = offset / 3; i < count; i += 3)
    {

        io_outb(0x03C8, i / 3);
        io_outb(0x03C9, c[i + 0]);
        io_outb(0x03C9, c[i + 1]);
        io_outb(0x03C9, c[i + 2]);

    }

    return 0;

}

static void start(struct base_driver *self)
{

    struct vga_driver *driver = (struct vga_driver *)self;
    unsigned int a;

    driver->cursor.color = (VGA_FB_COLOR_BLACK << 4) | (VGA_FB_COLOR_WHITE & 0x0F);

    for (a = 0; a < 80 * 25 * 2; a += 2)
    {

        ((char *)VGA_FB_ADDRESS)[a] = ' ';
        ((char *)VGA_FB_ADDRESS)[a + 1] = driver->cursor.color;

    }

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));
    base_init_driver(&driver->base, "vga", start, 0, 0);
    terminal_init_interface(&driver->terminal, &driver->base, read_terminal_data, write_terminal_data);
    video_init_interface(&driver->video, &driver->base, enable, read_video_data, write_video_data, read_video_colormap, write_video_colormap);

    driver->video.xres = 80;
    driver->video.yres = 25;
    driver->video.bpp = 16;

}

