#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/pci/pci.h>
#include "i915.h"

static struct base_driver driver;
static struct video_interface videointerface;
static struct arch_framebuffer *framebuffer = (struct arch_framebuffer *)ARCH_FIRMWARE_FRAMEBUFFER;
static unsigned short devices[I915_DEVICES] = {0x9A40, 0x9A49, 0x9A59, 0x9A60, 0x9A68, 0x9A70, 0x9A78, 0x9AC0, 0x9AC9, 0x9AD9, 0x9AF8};
static unsigned long mmio;
static unsigned int activewidth;
static unsigned int activeheight;

static unsigned int readreg(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void writereg(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

}

static unsigned int isdevice(unsigned short device)
{

    unsigned int i;

    for (i = 0; i < I915_DEVICES; i++)
    {

        if (devices[i] == device)
            return 1;

    }

    return 0;

}

static unsigned int ownsframebuffer(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 6; i++)
    {

        if (pci_getbar(id, i) == framebuffer->address)
            return 1;

    }

    return 0;

}

/* the firmware's mode: transcoder A lit, its plane linear XRGB8888 over the whole pipe, showing the firmware framebuffer */
static unsigned int checkfirmware(void)
{

    unsigned int control = readreg(I915_PLANE_CTL_1_A);
    unsigned int source = readreg(I915_PIPESRC_A);

    if (!(readreg(I915_TRANS_CONF_A) & I915_TRANS_CONF_ENABLE) || !(control & I915_PLANE_CTL_ENABLE))
        return 0;

    if ((control & I915_PLANE_CTL_FORMAT) != I915_PLANE_CTL_FORMAT_XRGB8888 || (control & I915_PLANE_CTL_TILED))
        return 0;

    if (readreg(I915_PS_CTRL_1_A) & I915_PS_CTRL_ENABLE)
        return 0;

    return (source >> 16) + 1 == framebuffer->width && (source & 0xFFFF) + 1 == framebuffer->height && activewidth == framebuffer->width && activeheight == framebuffer->height;

}

/* the pipe scaler stretches the source to the largest centered window with its aspect ratio; the rest shows the pipe's black background */
static void setscaler(unsigned int width, unsigned int height)
{

    unsigned int w = activewidth;
    unsigned int h = activewidth * height / width;

    if (h > activeheight)
    {

        h = activeheight;
        w = activeheight * width / height;

    }

    if (width == activewidth && height == activeheight)
    {

        writereg(I915_PS_CTRL_1_A, 0);
        writereg(I915_PS_WIN_POS_1_A, 0);
        writereg(I915_PS_WIN_SZ_1_A, 0);

    }

    else
    {

        writereg(I915_PS_CTRL_1_A, I915_PS_CTRL_ENABLE);
        writereg(I915_PS_WIN_POS_1_A, (((activewidth - w) / 2) << 16) | ((activeheight - h) / 2));
        writereg(I915_PS_WIN_SZ_1_A, (w << 16) | h);

    }

}

/* modes up to the firmware's size, width a multiple of 16 (the stride is counted in 64 bytes); others keep the current mode */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (mmio && bpp == 4 && width >= I915_MINSIZE && height >= I915_MINSIZE && width <= activewidth && height <= activeheight && !(width % 16))
    {

        writereg(I915_PIPESRC_A, ((width - 1) << 16) | (height - 1));
        setscaler(width, height);
        writereg(I915_PLANE_STRIDE_1_A, width * 4 / 64);
        writereg(I915_PLANE_SIZE_1_A, ((height - 1) << 16) | (width - 1));
        writereg(I915_PLANE_SURF_1_A, readreg(I915_PLANE_SURF_1_A));

        videointerface.width = width;
        videointerface.height = height;

    }

    arch_kmap(framebuffer->address, 0xA0000000, framebuffer->pitch * framebuffer->height, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, 0xA0000000, 0, videointerface_setmode);

    videointerface.width = framebuffer->width;
    videointerface.height = framebuffer->height;
    videointerface.bpp = 4;

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    if (type != PCI_BUS || !framebuffer->address || framebuffer->bpp != 32 || framebuffer->pitch != framebuffer->width * 4)
        return 0;

    return pci_inw(id, PCI_CONFIG_VENDOR) == I915_VENDOR && isdevice(pci_inw(id, PCI_CONFIG_DEVICE)) && ownsframebuffer(id);

}

/* without the firmware's mode as expected the driver only shows the firmware framebuffer, like gop */
static void driver_reset(unsigned int id)
{

    mmio = pci_getbar(id, 0);

    if (!mmio)
        return;

    arch_kmap(mmio, mmio, I915_MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    activewidth = (readreg(I915_TRANS_HTOTAL_A) & 0xFFFF) + 1;
    activeheight = (readreg(I915_TRANS_VTOTAL_A) & 0xFFFF) + 1;

    if (!checkfirmware())
        mmio = 0;

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

    base_initdriver(&driver, "i915", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

