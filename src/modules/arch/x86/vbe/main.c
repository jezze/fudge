#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/video/video.h>
#include <modules/arch/x86/pci/pci.h>
#include "vbe.h"

struct vbe_info
{

    unsigned int signature;
    unsigned short version;
    unsigned short oem[2];
    unsigned short capabilities[2];
    unsigned short video_modes_offset;
    unsigned short video_modes_segment;
    unsigned short video_memory;
    unsigned short software_rev;
    unsigned int vendor;
    unsigned int product_name;
    unsigned int product_rev;
    char reserved[222];
    char oem_data[256];

};

struct vbe_mode
{

    unsigned short attributes;
    unsigned short window_a;
    unsigned short granularity;
    unsigned short window_size;
    unsigned short segment_a;
    unsigned short segment_b;
    unsigned int win_func_ptr;
    unsigned short pitch;
    unsigned short width;
    unsigned short height;
    unsigned char w_char;
    unsigned char y_char;
    unsigned char planes;
    unsigned char bpp;
    unsigned char banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char image_pages;
    unsigned char reserved0;
    unsigned char red_mask;
    unsigned char red_position;
    unsigned char green_mask;
    unsigned char green_position;
    unsigned char blue_mask;
    unsigned char blue_position;
    unsigned char reserved_mask;
    unsigned char reserved_position;
    unsigned char direct_color_attributes;
    unsigned int framebuffer;
    unsigned int off_screen_mem_off;
    unsigned short off_screen_mem_size;

};

static struct base_driver driver;
static struct video_interface videointerface;
static struct vbe_mode *mode;

static unsigned short find(unsigned int w, unsigned int h, unsigned int bpp)
{

    struct vbe_info *info = vbe_getinfo();
    unsigned short *modes = (unsigned short *)((info->video_modes_segment * 16) + info->video_modes_offset);
    unsigned int i;

    for (i = 0; modes[i] != 0xFFFF; i++)
    {

        mode = vbe_getvideomode(modes[i]);

        if ((mode->attributes & 0x90) != 0x90)
            continue;

        if (mode->memory_model != 4 && mode->memory_model != 6)
            continue;

        if (mode->width == w && mode->height == h && mode->bpp == bpp)
            return modes[i];

    }

    return 0;

}

static void setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    unsigned short modenum;

    buffer_copy((void *)0xA000, (void *)(unsigned int)vbe_lowmemstart, (unsigned int)vbe_lowmemend - (unsigned int)vbe_lowmemstart);

    modenum = find(width, height, bpp);

    if (modenum)
    {

        if (mode->framebuffer)
        {

            arch_mapvideo(8, mode->framebuffer, mode->framebuffer, 0x00400000);
            arch_mapvideo(9, mode->framebuffer + 0x00400000, mode->framebuffer + 0x00400000, 0x00400000);

        }

        vbe_setvideomode(modenum | 0x4000);

        videointerface.width = mode->width;
        videointerface.height = mode->height;
        videointerface.bpp = mode->bpp / 8;

        video_notifymode(&videointerface, (void *)mode->framebuffer, videointerface.width, videointerface.height, videointerface.bpp);

    }

}

static unsigned int videointerface_notifyctrl(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    if (event == EVENT_CONFIG)
    {

        struct ctrl_videosettings *settings = data;

        setmode(settings->width, settings->height, settings->bpp * 8);

        return count;

    }

    return 0;

}

static unsigned int videointerface_readctrl(void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_videosettings settings;

    settings.width = videointerface.width;
    settings.height = videointerface.height;
    settings.bpp = videointerface.bpp;

    return buffer_read(buffer, count, &settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int videointerface_readdata(void *buffer, unsigned int count, unsigned int offset)
{

    return buffer_read(buffer, count, (void *)mode->framebuffer, videointerface.width * videointerface.height * videointerface.bpp, offset);

}

static unsigned int videointerface_writedata(void *buffer, unsigned int count, unsigned int offset)
{

    return buffer_write((void *)mode->framebuffer, videointerface.width * videointerface.height * videointerface.bpp, buffer, count, offset);

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id);

    videointerface.width = 80;
    videointerface.height = 25;
    videointerface.bpp = 2;
    videointerface.ctrl.operations.read = videointerface_readctrl;
    videointerface.ctrl.operations.notify = videointerface_notifyctrl;
    videointerface.data.operations.read = videointerface_readdata;
    videointerface.data.operations.write = videointerface_writedata;

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

    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "vbe", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

