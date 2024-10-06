#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
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
static struct {unsigned char color; unsigned int offset;} cursor = {0x0F, 0};
static struct vga_character *taddress = (struct vga_character *)0x000B8000;
/*
static unsigned int framebuffer = 0x000A0000;
*/

static void clear(unsigned int offset)
{

    unsigned int total = videointerface.width * videointerface.height;
    unsigned int i;

    for (i = offset; i < total; i++)
    {

        taddress[i].character = ' ';
        taddress[i].color = cursor.color;

    }

}

/*
static unsigned int consoleinterface_ondata(void *buffer, unsigned int count)
{

    unsigned int total = videointerface.width * videointerface.height;
    unsigned int i;

    if (videointerface.width != 80)
        return count;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            cursor.offset--;

        if (c == '\t')
            cursor.offset = (cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            cursor.offset -= (cursor.offset % videointerface.width);

        if (c == '\n')
            cursor.offset += videointerface.width - (cursor.offset % videointerface.width);

        if (c >= ' ')
        {

            taddress[cursor.offset].character = c;
            taddress[cursor.offset].color = cursor.color;
            cursor.offset++;

        }

        if (cursor.offset >= total)
        {

            buffer_copy(taddress, taddress + videointerface.width, videointerface.width * (videointerface.height - 1) * sizeof (struct vga_character));
            clear(videointerface.width * (videointerface.height - 1));

            cursor.offset -= videointerface.width;

        }

    }

    outcrt1(VGA_REG_CRTINDEX1_CRT0E, cursor.offset >> 8);
    outcrt1(VGA_REG_CRTINDEX1_CRT0F, cursor.offset);

    return MESSAGE_OK;

}
*/

/*
static unsigned int videointerface_getcmap(unsigned int source, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * 3; i += 3)
    {

        io_outb(VGA_REG_DACRINDEX, i / 3);
        c[i + 0] = io_inb(VGA_REG_DACDATA);
        c[i + 1] = io_inb(VGA_REG_DACDATA);
        c[i + 2] = io_inb(VGA_REG_DACDATA);

    }

    return i - offset;

}
*/

static unsigned int videointerface_onvideocmap(struct node *source, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    for (i = 0; i < count * 3; i += 3)
    {

        io_outb(VGA_REG_DACWINDEX, i / 3);
        io_outb(VGA_REG_DACDATA, c[i + 0]);
        io_outb(VGA_REG_DACDATA, c[i + 1]);
        io_outb(VGA_REG_DACDATA, c[i + 2]);

    }

    return MESSAGE_OK;

}

static unsigned int videointerface_onvideoconf(struct node *source, unsigned int width, unsigned int height, unsigned int bpp)
{

    if (width == 80)
    {

        if (videointerface.width == 320)
            vga_restore();

        videointerface.width = 80;
        videointerface.height = 25;
        videointerface.bpp = 2;

        vga_settext();

    }

    else
    {

        if (videointerface.width == 80)
            vga_save();

        videointerface.width = 320;
        videointerface.height = 200;
        videointerface.bpp = 1;

        vga_setgraphic();

    }

    video_notifymode(&videointerface, 0, videointerface.width, videointerface.height, videointerface.bpp);

    return MESSAGE_OK;

}

static void driver_init(unsigned int id)
{

    /*console_initinterface(&consoleinterface, id, consoleinterface_ondata);*/
    video_initinterface(&videointerface, id, videointerface_onvideocmap, videointerface_onvideoconf);

    videointerface.width = 80;
    videointerface.height = 25;
    videointerface.bpp = 2;

    clear(0);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "vga", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

