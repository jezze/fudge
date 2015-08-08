#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "registers.h"

#define VGA_COLORMAP_LIMIT              256

struct vga_character
{

    char character;
    char color;

};

static struct base_driver driver;
static struct console_interface consoleinterface;
static struct video_interface videointerface;
static struct {unsigned char color; unsigned short offset;} cursor;
static struct vga_character *taddress;
static void *gaddress;

static void clear(unsigned int offset)
{

    unsigned int total = videointerface.settings.w * videointerface.settings.h;
    unsigned int i;

    for (i = offset; i < total; i++)
    {

        taddress[i].character = ' ';
        taddress[i].color = cursor.color;

    }

}

static unsigned int consoleinterface_wout(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int total = videointerface.settings.w * videointerface.settings.h;
    unsigned int i;

    if (videointerface.settings.w != 80)
        return count;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            cursor.offset--;

        if (c == '\t')
            cursor.offset = (cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            cursor.offset -= (cursor.offset % videointerface.settings.w);

        if (c == '\n')
            cursor.offset += videointerface.settings.w - (cursor.offset % videointerface.settings.w);

        if (c >= ' ')
        {

            taddress[cursor.offset].character = c;
            taddress[cursor.offset].color = cursor.color;
            cursor.offset++;

        }

        if (cursor.offset >= total)
        {

            memory_read(taddress, videointerface.settings.w * videointerface.settings.h, taddress, videointerface.settings.w * videointerface.settings.h, sizeof (struct vga_character), videointerface.settings.w);
            clear(videointerface.settings.w * (videointerface.settings.h - 1));

            cursor.offset -= videointerface.settings.w;

        }

    }

    outcrt1(VGA_CRTINDEX_CRT0E, cursor.offset >> 8);
    outcrt1(VGA_CRTINDEX_CRT0F, cursor.offset);

    return count;

}

static void videointerface_setmode(struct ctrl_videosettings *settings)
{

    if (settings->w == 80)
    {

        if (videointerface.settings.w == 320)
            vga_restore();

        ctrl_setvideosettings(&videointerface.settings, 80, 25, 16);
        vga_settext();

    }

    else
    {

        if (videointerface.settings.w == 80)
            vga_save();

        ctrl_setvideosettings(&videointerface.settings, 320, 200, 8);
        vga_setgraphic();

    }

}

static unsigned int videointerface_rdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    unsigned int s = videointerface.settings.h * videointerface.settings.w * videointerface.settings.bpp / 8;

    return memory_read(buffer, count, gaddress, s, 1, offset);

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    /*
    unsigned int s = videointerface.settings.h * videointerface.settings.w * videointerface.settings.bpp / 8;
    */

    unsigned char *g = gaddress;
    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (b[i] != 0xFF)
            g[i + offset] = b[i];

    }

    return count;

}

static unsigned int videointerface_rcolormap(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (size != 3)
        return 0;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * size; i += size)
    {

        io_outb(VGA_REGISTER_DACRINDEX, i / size);
        c[i + 0] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 1] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 2] = io_inb(VGA_REGISTER_DACDATA);

    }

    return i - offset;

}

static unsigned int videointerface_wcolormap(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (size != 3)
        return 0;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * size; i += size)
    {

        io_outb(VGA_REGISTER_DACWINDEX, i / size);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return i - offset;

}

static void driver_init(void)
{

    taddress = (struct vga_character *)0x000B8000;
    gaddress = (void *)0x000A0000;
    cursor.color = 0x0F;

    console_initinterface(&consoleinterface, consoleinterface_wout);
    video_initinterface(&videointerface, videointerface_setmode, videointerface_rdata, videointerface_wdata, videointerface_rcolormap, videointerface_wcolormap);
    ctrl_setconsolesettings(&consoleinterface.settings, 1);
    ctrl_setvideosettings(&videointerface.settings, 80, 25, 16);
    clear(0);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface, id);
    video_registerinterface(&videointerface, id);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "vga", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

