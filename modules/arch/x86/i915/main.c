#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/video.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/pci/pci.h>
#include "i915.h"

static struct base_driver driver;
static struct base_video_interface ivideo;
static struct base_device device;
static struct base_video_node node;

static unsigned int read(unsigned int reg)
{

    volatile unsigned int *value = (volatile unsigned int *)(unsigned long)reg;

    return *value;

}

static void write(unsigned int reg, unsigned int val)
{

    volatile unsigned int *value = (volatile unsigned int *)(unsigned long)reg;

    *value = val;

}

static void wait(unsigned int num)
{

    unsigned int i = 0xcfffffff;
    unsigned int j = 0xcfffffff;

    while (j--)
    {

        while (i--);

    }

}

static void waitvblank()
{

    write(I915_PIPEB_STATUS, I915_PIPE_STATUS_VBLANK);

    while (!(read(I915_PIPEB_STATUS) & I915_PIPE_STATUS_VBLANK));

}

static void disablevga()
{

    write(I915_VGA_CONTROL, I915_VGA_CONTROL_DISABLE);

}

static void enabledpll()
{

    unsigned int pllb = read(I915_DPLLB_CONTROL);

    if (!(pllb & I915_DPLL_CONTROL_ENABLE_VCO))
    {

        write(I915_DPLLB_CONTROL, pllb);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);

    }

}

static void enablepipe()
{

    write(I915_PIPEB_CONFIG, read(I915_PIPEB_CONFIG) & I915_PIPE_CONFIG_ENABLE);

}

static void enableplane()
{

    write(I915_DISPLAYB_CONTROL, read(I915_DISPLAYB_CONTROL) & I915_DISPLAY_CONTROL_ENPLANE);

}

static void setpipemode(unsigned int width, unsigned int height)
{

    unsigned int htotal = (read(I915_DISPLAYB_HTOTAL) >> 16) + 1;
    unsigned int hdisplay = (read(I915_DISPLAYB_HTOTAL) & 0xFFFF) + 1;
    unsigned int hsyncs = (read(I915_DISPLAYB_HSYNC) >> 16) + 1;
    unsigned int hsynce = (read(I915_DISPLAYB_HSYNC) & 0xFFFF) + 1;
    unsigned int vtotal = (read(I915_DISPLAYB_VTOTAL) >> 16) + 1;
    unsigned int vdisplay = (read(I915_DISPLAYB_VTOTAL) & 0xFFFF) + 1;
    unsigned int vsyncs = (read(I915_DISPLAYB_VSYNC) >> 16) + 1;
    unsigned int vsynce = (read(I915_DISPLAYB_VSYNC) & 0xFFFF) + 1;

    write(I915_DISPLAYB_HTOTAL, ((htotal - 1) << 16) | ((hdisplay - 1)));
    write(I915_DISPLAYB_HBLANK, ((htotal - 1) << 16) | ((hdisplay - 1)));
    write(I915_DISPLAYB_HSYNC, ((hsyncs - 1) << 16) | ((hsynce - 1)));
    write(I915_DISPLAYB_VTOTAL, ((vtotal - 1) << 16) | ((vdisplay - 1)));
    write(I915_DISPLAYB_VBLANK, ((vtotal - 1) << 16) | ((vdisplay - 1)));
    write(I915_DISPLAYB_VSYNC, ((vsyncs - 1) << 16) | ((vsynce - 1)));

}

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static void ivideo_setmode(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp)
{

}

static unsigned int ivideo_rdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int ivideo_wdata(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int driver_check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inw(bus, id, PCI_CONFIG_VENDOR) == 0x8086 && pci_inw(bus, id, PCI_CONFIG_DEVICE) == 0x27AE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_initdevice(&device, bus, id);
    base_video_initnode(&node, &device, &ivideo);
    base_video_registernode(&node);
    pic_setroutine(bus, id, handleirq);
    enabledpll();
    enablepipe();
    enableplane();
    waitvblank();
    disablevga();
    setpipemode(640, 480);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    base_video_unregisternode(&node);

}

void init()
{

    base_video_initinterface(&ivideo, ivideo_setmode, ivideo_rdata, ivideo_wdata, 0, 0);
    base_video_registerinterface(&ivideo);
    base_initdriver(&driver, "i915", driver_check, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_video_unregisterinterface(&ivideo);
    base_unregisterdriver(&driver);

}

