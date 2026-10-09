#include <fudge.h>
#include <kernel.h>
#include <kernel/x86_64/cpu.h>
#include <kernel/x86_64/arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/arch/x86/pci/pci.h>

static struct base_driver driver;
static struct video_interface videointerface;
static struct arch_framebuffer *framebuffer = (struct arch_framebuffer *)ARCH_FIRMWARE_FRAMEBUFFER;

static void videointerface_oninfo(struct event_videoinfo *videoinfo)
{

    videoinfo->framebuffer = 0xA0000000;
    videoinfo->width = videointerface.width;
    videoinfo->height = videointerface.height;
    videoinfo->bpp = videointerface.bpp;

}

static void videointerface_onvideoconf(unsigned int width, unsigned int height, unsigned int bpp)
{

    arch_kmap(framebuffer->address, 0xA0000000, framebuffer->pitch * framebuffer->height, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

}

static unsigned int ownsframebuffer(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 6; i++)
    {

        unsigned int bar = pci_ind(id, PCI_CONFIG_BAR0 + i * 4);

        if (!(bar & 1) && (bar & 0xFFFFFFF0) == framebuffer->address)
            return 1;

    }

    return 0;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, videointerface_oninfo, 0, videointerface_onvideoconf);

    videointerface.width = framebuffer->width;
    videointerface.height = framebuffer->height;
    videointerface.bpp = framebuffer->bpp / 8;

}

static unsigned int driver_match(unsigned int id)
{

    if (!framebuffer->address || (framebuffer->bpp != 24 && framebuffer->bpp != 32) || framebuffer->pitch != framebuffer->width * (framebuffer->bpp / 8))
        return 0;

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && ownsframebuffer(id);

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

