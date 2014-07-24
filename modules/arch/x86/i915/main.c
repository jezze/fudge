#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <base/video.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/pci/pci.h>
#include "i915.h"

static struct base_driver driver;
static struct base_video_interface ivideo;

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

static void wait_vblank()
{

    write(I915_PIPEB_STATUS, I915_PIPE_STATUS_VBLANK);

    while (!(read(I915_PIPEB_STATUS) & I915_PIPE_STATUS_VBLANK));

}

static void disable_vga()
{

    write(I915_VGA_CONTROL, I915_VGA_CONTROL_DISABLE);

}

static void enable_dpll()
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

static void enable_pipe()
{

    write(I915_PIPEB_CONFIG, read(I915_PIPEB_CONFIG) & I915_PIPE_CONFIG_ENABLE);

}

static void enable_plane()
{

    write(I915_DISPLAYB_CONTROL, read(I915_DISPLAYB_CONTROL) & I915_DISPLAY_CONTROL_ENPLANE);

}

static void set_pipe_mode(unsigned int width, unsigned int height)
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

static void enable(struct base_bus *bus, unsigned int id)
{

}

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static void handle_irq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_bus_inw(bus, id, PCI_CONFIG_VENDOR) == 0x8086 && pci_bus_inw(bus, id, PCI_CONFIG_DEVICE) == 0x27AE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    base_video_connect_interface(&ivideo, bus, id);
    pic_set_routine(irq, bus, id, handle_irq);
    enable_dpll();
    enable_pipe();
    enable_plane();
    wait_vblank();
    disable_vga();
    set_pipe_mode(640, 480);

}

static void detach(struct base_bus *bus, unsigned int id)
{

    unsigned short irq = bus->device_irq(bus, id);

    pic_unset_routine(irq, bus, id);

}

void init()
{

    base_video_init_interface(&ivideo, enable, read_data, write_data, 0, 0);
    base_video_register_interface(&ivideo);
    base_init_driver(&driver, "i915", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_video_unregister_interface(&ivideo);
    base_unregister_driver(&driver);

}

