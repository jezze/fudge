#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "registers.h"

#define VGA_TEXT_LIMIT                  2000
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
static void *taddress;
static void *gaddress;

static void clear(unsigned int offset)
{

    struct vga_character *memory = taddress;
    unsigned int i;

    for (i = offset; i < VGA_TEXT_LIMIT; i++)
    {

        memory[i].character = ' ';
        memory[i].color = cursor.color;

    }

}

static unsigned int consoleinterface_wout(unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_character *memory = taddress;
    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;
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
            cursor.offset -= (cursor.offset % 80);

        if (c == '\n')
            cursor.offset += 80 - (cursor.offset % 80);

        if (c >= ' ')
        {

            memory[cursor.offset].character = c;
            memory[cursor.offset].color = cursor.color;
            cursor.offset++;

        }

        if (cursor.offset >= VGA_TEXT_LIMIT)
        {

            memory_read(taddress, fullsize, taddress, fullsize, 1, linesize);
            clear(80 * 24);
            cursor.offset -= 80;

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

    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;

    return memory_read(buffer, count, gaddress, fullsize, size, offset);

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;

    return memory_write(gaddress, fullsize, buffer, count, size, offset);

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

static void driver_init()
{

    taddress = (void *)0x000B8000;
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

void module_init()
{

    base_initdriver(&driver, "vga", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PCI_BUS);

}

