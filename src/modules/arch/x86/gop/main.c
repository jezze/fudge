#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/base/driver.h>
#include <modules/video/video.h>
#include <modules/arch/x86/pci/pci.h>

static struct base_driver driver;
static struct video_interface videointerface;

static void videointerface_oninfo(struct event_videoinfo *videoinfo)
{

    videoinfo->framebuffer = 0xA0000000;
    videoinfo->width = videointerface.width;
    videoinfo->height = videointerface.height;
    videoinfo->bpp = videointerface.bpp;

}

static void videointerface_onvideoconf(unsigned int width, unsigned int height, unsigned int bpp)
{

    unsigned long framebuffer = *(unsigned long *)0x10000;

    arch_kmap(framebuffer, 0xA0000000, videointerface.width * videointerface.height * videointerface.bpp, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, videointerface_oninfo, 0, videointerface_onvideoconf);

    videointerface.width = 1024;
    videointerface.height = 768;
    videointerface.bpp = 4;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY;

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

    base_initdriver(&driver, "gop", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

