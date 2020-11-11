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

extern void *realmode_gdt;
extern short modenum;

static struct base_driver driver;
static struct video_interface videointerface;
static unsigned int framebuffer;

static void run(unsigned int w, unsigned int h, unsigned int bpp)
{

    struct vbe_info *info = (struct vbe_info *)0xC000;
    struct vbe_mode *mode = (struct vbe_mode *)0xD000;
    unsigned short *modes;
    unsigned int i;

    debug_logs(DEBUG_INFO, "vbe loaded");
    memory_copy((void *)0x9000, &realmode_gdt, 0x1000);
    memory_copy((void *)0x8000, (void *)(unsigned int)vbe_begin16, (unsigned int)vbe_end16 - (unsigned int)vbe_begin16);

    vbe_getedid();
    vbe_getinfo();
    debug_logs(DEBUG_INFO, "vbe worked!");
    debug_log16(DEBUG_INFO, "vbe version", info->version);
    debug_log16(DEBUG_INFO, "vbe video segment", info->video_modes_segment);
    debug_log16(DEBUG_INFO, "vbe video offset", info->video_modes_offset);

    modes = (unsigned short *)((info->video_modes_segment * 16) + info->video_modes_offset);

    for (i = 0; modes[i] != 0xFFFF; i++)
    {

        modenum = modes[i];

        vbe_getvideomode();

        if (mode->width == w && mode->height == h && mode->bpp == bpp)
        {

            debug_log16(DEBUG_INFO, "vbe modenum", modenum);
            debug_log16(DEBUG_INFO, "vbe width", mode->width);
            debug_log16(DEBUG_INFO, "vbe height", mode->height);
            debug_log8(DEBUG_INFO, "vbe bpp", mode->bpp);
            debug_log32(DEBUG_INFO, "vbe framebuffer", mode->framebuffer);
            debug_log8(DEBUG_INFO, "vbe memory_model", mode->memory_model);

            break;

        }

    }

    if (modenum == 0xFFFF)
        return;

    ctrl_setvideosettings(&videointerface.settings, mode->width, mode->height, mode->bpp / 8);

    framebuffer = mode->framebuffer;

    if (framebuffer)
    {

        arch_setmapvideo(4, framebuffer, framebuffer, 0x00400000);
        arch_setmapvideo(5, framebuffer + 0x00400000, framebuffer + 0x00400000, 0x00400000);

    }

    /* Set mode */
    modenum |= 0x4000;

    vbe_setvideomode();

    video_notifymode(&videointerface, framebuffer, videointerface.settings.w, videointerface.settings.h, videointerface.settings.bpp);

}

static unsigned int videointerface_readctrl(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &videointerface.settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int videointerface_writectrl(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_videosettings *settings = buffer;

    run(settings->w, settings->h, settings->bpp * 8);

    return count;

}

static unsigned int videointerface_readdata(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, (void *)framebuffer, videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp, offset);

}

static unsigned int videointerface_writedata(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write((void *)framebuffer, videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp, buffer, count, offset);

}

static unsigned int videointerface_readcolormap(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int videointerface_writecolormap(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return count;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id);
    ctrl_setvideosettings(&videointerface.settings, 80, 25, 2);

    videointerface.ctrl.operations.read = videointerface_readctrl;
    videointerface.ctrl.operations.write = videointerface_writectrl;
    videointerface.data.operations.read = videointerface_readdata;
    videointerface.data.operations.write = videointerface_writedata;
    videointerface.colormap.operations.read = videointerface_readcolormap;
    videointerface.colormap.operations.write = videointerface_writecolormap;

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

