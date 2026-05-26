#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/console/console.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "registers.h"

#define VGA_COLORMAP_LIMIT              256

static struct base_driver driver;
static struct console_interface consoleinterface;
static struct video_interface videointerface;
static unsigned char *consoledata = (unsigned char *)0x000B8000;
/*
static unsigned char *videodata = (unsigned char *)0x000A0000;
*/

static void print(char c)
{

    consoledata[consoleinterface.cursor * 2] = c;
    consoledata[consoleinterface.cursor * 2 + 1] = consoleinterface.color;

}

static void clearafter(unsigned int offset)
{

    unsigned int current = consoleinterface.cursor;
    unsigned int total = consoleinterface.width * consoleinterface.height;
    unsigned int i;

    console_cursorset(&consoleinterface, offset);

    for (i = offset; i < total; i++)
    {

        print(' ');
        console_cursorright(&consoleinterface, 1);

    }

    console_cursorset(&consoleinterface, current);

}

static void scrollup(unsigned int lines)
{

    buffer_copy(consoledata, consoledata + (consoleinterface.width * lines), consoleinterface.width * (consoleinterface.height - lines) * 2);

}

static void checkscroll(void)
{

    if (consoleinterface.cursor >= consoleinterface.width * consoleinterface.height)
    {

        scrollup(1);
        clearafter(consoleinterface.width * (consoleinterface.height - 1));
        console_cursorup(&consoleinterface, 1);

    }

}

static void handle(char c)
{

    switch (c)
    {

    case '\b':
        console_cursorleft(&consoleinterface, 1);

        break;

    case '\t':
        console_cursorright(&consoleinterface, consoleinterface.cursor + 8 - (consoleinterface.cursor % 8));

        break;

    case '\r':
        console_cursorhome(&consoleinterface);

        break;

    case '\n':
        console_cursordown(&consoleinterface, 1);

        break;

    default:
        print(c);
        console_cursorright(&consoleinterface, 1);

        break;

    }

}

static unsigned int consoleinterface_ondata(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        handle(b[i]);
        checkscroll();

    }

    outcrt1(VGA_REG_CRTINDEX1_CRT0E, consoleinterface.cursor >> 8);
    outcrt1(VGA_REG_CRTINDEX1_CRT0F, consoleinterface.cursor);

    return MESSAGE_OK;

}

/*
static unsigned int videointerface_getcmap(unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count * 3; i += 3)
    {

        io_outb(VGA_REG_DACRINDEX, i / 3);
        b[i + 0] = io_inb(VGA_REG_DACDATA);
        b[i + 1] = io_inb(VGA_REG_DACDATA);
        b[i + 2] = io_inb(VGA_REG_DACDATA);

    }

    return i - offset;

}
*/

static unsigned int videointerface_onvideocmap(unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    for (i = 0; i < count * 3; i += 3)
    {

        io_outb(VGA_REG_DACWINDEX, i / 3);
        io_outb(VGA_REG_DACDATA, b[i + 0]);
        io_outb(VGA_REG_DACDATA, b[i + 1]);
        io_outb(VGA_REG_DACDATA, b[i + 2]);

    }

    return MESSAGE_OK;

}

static unsigned int videointerface_onvideoconf(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp)
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

    console_initinterface(&consoleinterface, id, consoleinterface_ondata);
    video_initinterface(&videointerface, id, videointerface_onvideocmap, videointerface_onvideoconf);

    consoleinterface.width = 80;
    consoleinterface.height = 25;
    consoleinterface.color = 0x0F;
    videointerface.width = 80;
    videointerface.height = 25;
    videointerface.bpp = 2;

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

