#include <fudge/module.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "vga.h"

#define VGA_ADDRESS                     0x000A0000
#define VGA_TEXT_ADDRESS                0x000B8000
#define VGA_TEXT_SIZE                   2000

enum vga_register
{

    VGA_REGISTER_CRMINDEX               = 0x03B4,
    VGA_REGISTER_CRMDATA                = 0x03B5,
    VGA_REGISTER_FCMCTRL                = 0x03BA,
    VGA_REGISTER_ARINDEX                = 0x03C0,
    VGA_REGISTER_ARWRITE                = 0x03C0,
    VGA_REGISTER_ARREAD                 = 0x03C1,
    VGA_REGISTER_MISCWRITE              = 0x03C2,
    VGA_REGISTER_SRINDEX                = 0x03C4,
    VGA_REGISTER_SRDATA                 = 0x03C5,
    VGA_REGISTER_DACPIXMASK             = 0x03C6,
    VGA_REGISTER_DACRINDEX              = 0x03C7,
    VGA_REGISTER_DACWINDEX              = 0x03C8,
    VGA_REGISTER_DACDATA                = 0x03C9,
    VGA_REGISTER_FCSTAT                 = 0x03CA,
    VGA_REGISTER_MISCREAD               = 0x03CC,
    VGA_REGISTER_GRINDEX                = 0x03CE,
    VGA_REGISTER_GRDATA                 = 0x03CF,
    VGA_REGISTER_CRCINDEX               = 0x03D4,
    VGA_REGISTER_CRCDATA                = 0x03D5,
    VGA_REGISTER_FCCCTRL                = 0x03DA

};

enum vga_arindex
{

    VGA_ARINDEX_AR00                    = 0x00,
    VGA_ARINDEX_AR01                    = 0x01,
    VGA_ARINDEX_AR02                    = 0x02,
    VGA_ARINDEX_AR03                    = 0x03,
    VGA_ARINDEX_AR04                    = 0x04,
    VGA_ARINDEX_AR05                    = 0x05,
    VGA_ARINDEX_AR06                    = 0x06,
    VGA_ARINDEX_AR07                    = 0x07,
    VGA_ARINDEX_AR08                    = 0x08,
    VGA_ARINDEX_AR09                    = 0x09,
    VGA_ARINDEX_AR0A                    = 0x0A,
    VGA_ARINDEX_AR0B                    = 0x0B,
    VGA_ARINDEX_AR0C                    = 0x0C,
    VGA_ARINDEX_AR0D                    = 0x0D,
    VGA_ARINDEX_AR0E                    = 0x0E,
    VGA_ARINDEX_AR0F                    = 0x0F,
    VGA_ARINDEX_AR10                    = 0x10,
    VGA_ARINDEX_AR11                    = 0x11,
    VGA_ARINDEX_AR12                    = 0x12,
    VGA_ARINDEX_AR13                    = 0x13,
    VGA_ARINDEX_AR14                    = 0x14,
    VGA_ARINDEX_ENABLE                  = (1 << 5)

};

enum vga_ar10
{

    VGA_AR10_GRAPHICS                   = (1 << 0),
    VGA_AR10_MDA                        = (1 << 1),
    VGA_AR10_LINE                       = (1 << 2),
    VGA_AR10_BLINK                      = (1 << 3),
    VGA_AR10_PIXPAN                     = (1 << 5),
    VGA_AR10_PIXCLK                     = (1 << 6),
    VGA_AR10_ENABLE14                   = (1 << 7)

};

enum vga_crindex
{

    VGA_CRINDEX_CR00                    = 0x00,
    VGA_CRINDEX_CR01                    = 0x01,
    VGA_CRINDEX_CR02                    = 0x02,
    VGA_CRINDEX_CR03                    = 0x03,
    VGA_CRINDEX_CR04                    = 0x04,
    VGA_CRINDEX_CR05                    = 0x05,
    VGA_CRINDEX_CR06                    = 0x06,
    VGA_CRINDEX_CR07                    = 0x07,
    VGA_CRINDEX_CR08                    = 0x08,
    VGA_CRINDEX_CR09                    = 0x09,
    VGA_CRINDEX_CR0A                    = 0x0A,
    VGA_CRINDEX_CR0B                    = 0x0B,
    VGA_CRINDEX_CR0C                    = 0x0C,
    VGA_CRINDEX_CR0D                    = 0x0D,
    VGA_CRINDEX_CR0E                    = 0x0E,
    VGA_CRINDEX_CR0F                    = 0x0F,
    VGA_CRINDEX_CR10                    = 0x10,
    VGA_CRINDEX_CR11                    = 0x11,
    VGA_CRINDEX_CR12                    = 0x12,
    VGA_CRINDEX_CR13                    = 0x13,
    VGA_CRINDEX_CR14                    = 0x14,
    VGA_CRINDEX_CR15                    = 0x15,
    VGA_CRINDEX_CR16                    = 0x16,
    VGA_CRINDEX_CR17                    = 0x17,
    VGA_CRINDEX_CR18                    = 0x18,
    VGA_CRINDEX_CR22                    = 0x22,
    VGA_CRINDEX_CR24                    = 0x24,
    VGA_CRINDEX_CR26                    = 0x26

};

enum vga_grindex
{

    VGA_GRINDEX_GR00                    = 0x00,
    VGA_GRINDEX_GR01                    = 0x01,
    VGA_GRINDEX_GR02                    = 0x02,
    VGA_GRINDEX_GR03                    = 0x03,
    VGA_GRINDEX_GR04                    = 0x04,
    VGA_GRINDEX_GR05                    = 0x05,
    VGA_GRINDEX_GR06                    = 0x06,
    VGA_GRINDEX_GR07                    = 0x07,
    VGA_GRINDEX_GR08                    = 0x08

};

enum vga_gr03
{

    VGA_GR03_NONE                       = (0 << 3),
    VGA_GR03_AND                        = (1 << 3),
    VGA_GR03_OR                         = (2 << 3),
    VGA_GR03_XOR                        = (3 << 3)

};

enum vga_gr05
{

    VGA_GR05_WMODE0                     = (0 << 0),
    VGA_GR05_WMODE1                     = (1 << 0),
    VGA_GR05_WMODE2                     = (2 << 0),
    VGA_GR05_WMODE3                     = (3 << 0),
    VGA_GR05_RMODE0                     = (0 << 3),
    VGA_GR05_RMODE1                     = (1 << 3),
    VGA_GR05_ODDEVEN                    = (1 << 4),
    VGA_GR05_SHIFTMODE                  = (1 << 5),
    VGA_GR05_256MODE                    = (1 << 6)

};

enum vga_gr06
{

    VGA_GR06_GRAPHICS                   = (1 << 0),
    VGA_GR06_CHAIN                      = (1 << 1),
    VGA_GR06_EXTENDED                   = (0 << 2),
    VGA_GR06_EGAVGA                     = (1 << 2),
    VGA_GR06_HERCULES                   = (2 << 2),
    VGA_GR06_CGA                        = (3 << 2)

};

enum vga_miscwrite
{

    VGA_MISCWRITE_MONO                  = (0 << 0),
    VGA_MISCWRITE_COLOR                 = (1 << 0),
    VGA_MISCWRITE_DISABLE               = (0 << 1),
    VGA_MISCWRITE_ENABLE                = (1 << 1),
    VGA_MISCWRITE_VCLK0                 = (0 << 2),
    VGA_MISCWRITE_VCLK1                 = (1 << 2),
    VGA_MISCWRITE_VCLK2                 = (2 << 2),
    VGA_MISCWRITE_VCLK3                 = (3 << 2),
    VGA_MISCWRITE_PAGESELECT            = (1 << 5),
    VGA_MISCWRITE_SYNC400               = (1 << 6),
    VGA_MISCWRITE_SYNC350               = (2 << 6),
    VGA_MISCWRITE_SYNC480               = (3 << 6)

};

enum vga_srindex
{

    VGA_SRINDEX_SR00                    = 0x00,
    VGA_SRINDEX_SR01                    = 0x01,
    VGA_SRINDEX_SR02                    = 0x02,
    VGA_SRINDEX_SR03                    = 0x03,
    VGA_SRINDEX_SR04                    = 0x04

};

enum vga_sr00
{

    VGA_SR00_ASYNC                      = (1 << 0),
    VGA_SR00_SYNC                       = (1 << 1)

};

enum vga_sr01
{

    VGA_SR01_DIV9                       = (0 << 0),
    VGA_SR01_DIV8                       = (1 << 0),
    VGA_SR01_SHIFT16                    = (1 << 2),
    VGA_SR01_DIV2                       = (1 << 3),
    VGA_SR01_SHIFT32                    = (1 << 4),
    VGA_SR01_FULL                       = (1 << 5)

};

enum vga_sr02
{

    VGA_SR02_ENABLE0                    = (1 << 0),
    VGA_SR02_ENABLE1                    = (1 << 1),
    VGA_SR02_ENABLE2                    = (1 << 2),
    VGA_SR02_ENABLE3                    = (1 << 3)

};

enum vga_sr03
{

    VGA_SR03_P0K                        = (0 << 0),
    VGA_SR03_P16K                       = (1 << 0),
    VGA_SR03_P32K                       = (2 << 0),
    VGA_SR03_P48K                       = (3 << 0),
    VGA_SR03_P8K                        = ((1 << 4) | VGA_SR03_P0K),
    VGA_SR03_P24K                       = ((1 << 4) | VGA_SR03_P16K),
    VGA_SR03_P40K                       = ((1 << 4) | VGA_SR03_P32K),
    VGA_SR03_P56K                       = ((1 << 4) | VGA_SR03_P48K),
    VGA_SR03_S0K                        = (0 << 2),
    VGA_SR03_S16K                       = (1 << 2),
    VGA_SR03_S32K                       = (2 << 2),
    VGA_SR03_S48K                       = (3 << 2),
    VGA_SR03_S8K                        = ((1 << 5) | VGA_SR03_S0K),
    VGA_SR03_S24K                       = ((1 << 5) | VGA_SR03_S16K),
    VGA_SR03_S40K                       = ((1 << 5) | VGA_SR03_S32K),
    VGA_SR03_S56K                       = ((1 << 5) | VGA_SR03_S48K)

};

enum vga_sr04
{

    VGA_SR04_EXTENDED                   = (1 << 1),
    VGA_SR04_ODDEVEN                    = (1 << 2),
    VGA_SR04_CHAIN                      = (1 << 3)

};

static unsigned char windex[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x13};
static unsigned char w256[] = {0x5F, 0x3F, 0x40, 0x82, 0x4A, 0x9A, 0x20};
static unsigned char w320[] = {0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x28};
static unsigned char w360[] = {0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0x2D};
static unsigned char w376[] = {0x6E, 0x5D, 0x5E, 0x91, 0x62, 0x8F, 0x2F};
static unsigned char w400[] = {0x70, 0x63, 0x64, 0x92, 0x65, 0x82, 0x32};
static unsigned char hindex[] = {0x06, 0x07, 0x09, 0x10, 0x11, 0x12, 0x15, 0x16};
static unsigned char h200[] = {0xBF, 0x1F, 0x41, 0x9C, 0x8E, 0x8F, 0x96, 0xB9};
static unsigned char h224[] = {0x0B, 0x3E, 0x41, 0xDA, 0x9C, 0xBF, 0xC7, 0x04};
static unsigned char h240[] = {0x0D, 0x3E, 0x41, 0xEA, 0xAC, 0xDF, 0xE7, 0x06};
static unsigned char h256[] = {0x23, 0xB2, 0x61, 0x0A, 0xAC, 0xFF, 0x07, 0x1A};
static unsigned char h270[] = {0x30, 0xF0, 0x61, 0x20, 0xA9, 0x1B, 0x1F, 0x2F};
static unsigned char h300[] = {0x70, 0xF0, 0x61, 0x5B, 0x8C, 0x57, 0x58, 0x70};
static unsigned char h360[] = {0xBF, 0x1F, 0x40, 0x88, 0x85, 0x67, 0x6D, 0xBA};
static unsigned char h400[] = {0xBF, 0x1F, 0x40, 0x9C, 0x8E, 0x8F, 0x96, 0xB9};
static unsigned char h480[] = {0x0D, 0x3E, 0x40, 0xEA, 0xAC, 0xDF, 0xE7, 0x06};
static unsigned char h564[] = {0x62, 0xF0, 0x60, 0x37, 0x89, 0x33, 0x3C, 0x5C};
static unsigned char h600[] = {0x70, 0xF0, 0x60, 0x5B, 0x8C, 0x57, 0x58, 0x70};

static void write16i(unsigned short port, unsigned char index, unsigned char value)
{

    io_outw(port, (value << 8) | index);

}

static void mode(struct video_interface *self)
{

    unsigned int chain4 = 1;
    unsigned char misc = VGA_MISCWRITE_COLOR | VGA_MISCWRITE_ENABLE | VGA_MISCWRITE_PAGESELECT;
    unsigned char *w;
    unsigned char *h;
    unsigned int a;

    if (chain4 && self->xres * self->yres > 65536)
        return;

    switch (self->xres)
    {

        case 256:
            w = w256;
            misc |= VGA_MISCWRITE_VCLK0;

            break;

        case 320:
            w = w320;
            misc |= VGA_MISCWRITE_VCLK0;

            break;

        case 360:
            w = w360;
            misc |= VGA_MISCWRITE_VCLK1;

            break;

        case 376:
            w = w376;
            misc |= VGA_MISCWRITE_VCLK1;

            break;

        case 400:
            w = w400;
            misc |= VGA_MISCWRITE_VCLK1;

            break;

        default:
            return;

    }

    switch (self->yres)
    {

        case 200:
            h = h200;
            misc |= VGA_MISCWRITE_SYNC400;

            break;

        case 224:
            h = h224;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 240:
            h = h240;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 256:
            h = h256;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 270:
            h = h270;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 300:
            h = h300;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 360:
            h = h360;
            misc |= VGA_MISCWRITE_SYNC400;

            break;

        case 400:
            h = h400;
            misc |= VGA_MISCWRITE_SYNC400;

            break;

        case 480:
            h = h480;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 564:
            h = h564;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        case 600:
            h = h600;
            misc |= VGA_MISCWRITE_SYNC480;

            break;

        default:
            return;

    }

    io_outb(VGA_REGISTER_MISCWRITE, misc);

    for (a = 0; a < 7; a++)
        write16i(VGA_REGISTER_CRCINDEX, windex[a], w[a]);

    for (a = 0; a < 8; a++)
        write16i(VGA_REGISTER_CRCINDEX, hindex[a], h[a]);

    if (chain4)
    {
    
        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR14, 0x40);
        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR17, 0xA3);
        write16i(VGA_REGISTER_SRINDEX, VGA_SRINDEX_SR04, VGA_SR04_EXTENDED | VGA_SR04_ODDEVEN | VGA_SR04_CHAIN);

    }

    else
    {

        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR14, 0x00);
        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR17, 0xE3);
        write16i(VGA_REGISTER_SRINDEX, VGA_SRINDEX_SR04, VGA_SR04_EXTENDED | VGA_SR04_ODDEVEN);

    }

    write16i(VGA_REGISTER_SRINDEX, VGA_SRINDEX_SR01, VGA_SR01_DIV8);
    write16i(VGA_REGISTER_SRINDEX, VGA_SRINDEX_SR02, VGA_SR02_ENABLE0 | VGA_SR02_ENABLE1 | VGA_SR02_ENABLE2 | VGA_SR02_ENABLE3);
    write16i(VGA_REGISTER_GRINDEX, VGA_GRINDEX_GR05, VGA_GR05_256MODE);
    write16i(VGA_REGISTER_GRINDEX, VGA_GRINDEX_GR06, VGA_GR06_GRAPHICS | VGA_GR06_EGAVGA);
    io_inb(VGA_REGISTER_FCCCTRL);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_AR10);
    io_outb(VGA_REGISTER_ARINDEX, VGA_AR10_GRAPHICS | VGA_AR10_PIXCLK);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_AR13);
    io_outb(VGA_REGISTER_ARINDEX, 0x00);

    for (a = 0; a < 16; a++)
    {

        io_outb(VGA_REGISTER_ARINDEX, a);
        io_outb(VGA_REGISTER_ARINDEX, a);

    }

    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_ENABLE);

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

            ((char *)VGA_TEXT_ADDRESS)[driver->cursor.offset * 2] = c;
            ((char *)VGA_TEXT_ADDRESS)[driver->cursor.offset * 2 + 1] = driver->cursor.color;

            driver->cursor.offset++;

        }

        if (driver->cursor.offset >= VGA_TEXT_SIZE)
        {

            unsigned int a;

            memory_copy((void *)VGA_TEXT_ADDRESS, (void *)(VGA_TEXT_ADDRESS + 80 * 2), 80 * 24 * 2);

            for (a = 80 * 24 * 2; a < 80 * 25 * 2; a += 2)
            {

                ((char *)VGA_TEXT_ADDRESS)[a] = ' ';
                ((char *)VGA_TEXT_ADDRESS)[a + 1] = driver->cursor.color;

            }

            driver->cursor.offset -= 80;

        }

        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR0E, driver->cursor.offset >> 8);
        write16i(VGA_REGISTER_CRCINDEX, VGA_CRINDEX_CR0F, driver->cursor.offset);

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

        io_outb(VGA_REGISTER_DACWINDEX, i / 3);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return 0;

}

static void start(struct base_driver *self)
{

    struct vga_driver *driver = (struct vga_driver *)self;
    unsigned int a;

    driver->cursor.color = 0x0F;

    for (a = 0; a < 80 * 25 * 2; a += 2)
    {

        ((char *)VGA_TEXT_ADDRESS)[a] = ' ';
        ((char *)VGA_TEXT_ADDRESS)[a + 1] = driver->cursor.color;

    }

}

static void attach(struct base_device *device)
{

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inb(pciDevice, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_device_inb(pciDevice, PCI_CONFIG_SUBCLASS) == 0x00 && pci_device_inb(pciDevice, PCI_CONFIG_INTERFACE) == 0x00;

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));
    base_init_driver(&driver->base, "vga", start, check, attach);
    terminal_init_interface(&driver->iterminal, &driver->base, read_terminal_data, write_terminal_data);
    video_init_interface(&driver->ivideo, &driver->base, mode, read_video_data, write_video_data, read_video_colormap, write_video_colormap);

    driver->ivideo.xres = 80;
    driver->ivideo.yres = 25;
    driver->ivideo.bpp = 16;

}

