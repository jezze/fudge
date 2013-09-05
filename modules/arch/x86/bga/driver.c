#include <fudge/module.h>
#include <x86/kernel/mmu.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "bga.h"

#define BGA_PCI_VENDOR                  0x1234
#define BGA_PCI_DEVICE                  0x1111

enum bga_register
{

    BGA_REGISTER_COMMAND                = 0x01CE,
    BGA_REGISTER_DATA                   = 0x01CF

};

enum bga_command
{

    BGA_COMMAND_ID                      = 0x0000,
    BGA_COMMAND_XRES                    = 0x0001,
    BGA_COMMAND_YRES                    = 0x0002,
    BGA_COMMAND_BPP                     = 0x0003,
    BGA_COMMAND_ENABLE                  = 0x0004,
    BGA_COMMAND_BANK                    = 0x0005,
    BGA_COMMAND_VIRT_WIDTH              = 0x0006,
    BGA_COMMAND_VIRT_HEIGHT             = 0x0007,
    BGA_COMMAND_XOFF                    = 0x0008,
    BGA_COMMAND_YOFF                    = 0x0009

};

enum bga_bpp
{

    BGA_BPP_4                           = 0x04,
    BGA_BPP_8                           = 0x08,
    BGA_BPP_15                          = 0x0F,
    BGA_BPP_16                          = 0x10,
    BGA_BPP_24                          = 0x18,
    BGA_BPP_32                          = 0x20

};

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_REGISTER_COMMAND, index);
    io_outw(BGA_REGISTER_DATA, data);

}

static void start(struct base_driver *self)
{

/*
    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_map_kernel_memory(3, (unsigned int)driver->lfb, (unsigned int)driver->lfb, 0x00400000);
    mmu_reload_memory();
*/

}

static void attach(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;
    struct bga_driver *driver = (struct bga_driver *)device->driver;

    driver->lfb = (void *)pci_device_ind(pciDevice, PCI_CONFIG_BAR0);

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inw(pciDevice, PCI_CONFIG_VENDOR) == BGA_PCI_VENDOR && pci_device_inw(pciDevice, PCI_CONFIG_DEVICE) == BGA_PCI_DEVICE;

}

static void mode(struct video_interface *self)
{

    write_register(BGA_COMMAND_ENABLE, 0x00);
    write_register(BGA_COMMAND_XRES, self->xres);
    write_register(BGA_COMMAND_YRES, self->yres);
    write_register(BGA_COMMAND_BPP, self->bpp);
    write_register(BGA_COMMAND_ENABLE, 0x40 | 0x01);

}

static unsigned int read_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;
    unsigned int size = self->xres * self->yres * self->bpp / 4;

    return memory_read(buffer, count, driver->lfb, size, offset);

}

static unsigned int write_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)self->driver;
    unsigned int size = self->xres * self->yres * self->bpp / 4;

    return memory_write(driver->lfb, size, buffer, count, offset);

}

void bga_init_driver(struct bga_driver *driver)
{

    memory_clear(driver, sizeof (struct bga_driver));
    base_init_driver(&driver->base, "bga", start, check, attach);
    video_init_interface(&driver->ivideo, &driver->base, mode, read_data, write_data, 0, 0);

    driver->ivideo.xres = 800;
    driver->ivideo.yres = 600;
    driver->ivideo.bpp = BGA_BPP_32;
    driver->bank = (void *)0xA0000;

}

