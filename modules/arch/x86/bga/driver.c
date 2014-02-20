#include <module.h>
#include <x86/arch/mmu.h>
#include <base/base.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "driver.h"

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

static void attach(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;
    struct bga_driver *driver = (struct bga_driver *)device->driver;

    base_video_register_interface(&driver->ivideo, device);

    driver->lfb = (void *)(unsigned long)pci_device_ind(pciDevice, PCI_CONFIG_BAR0);

/*
    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_map_kernel_memory(3, (unsigned int)driver->lfb, (unsigned int)driver->lfb, 0x00400000);
    mmu_reload_memory();
*/

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inw(pciDevice, PCI_CONFIG_VENDOR) == BGA_PCI_VENDOR && pci_device_inw(pciDevice, PCI_CONFIG_DEVICE) == BGA_PCI_DEVICE;

}

static void mode(struct base_device *device)
{

    struct bga_driver *driver = (struct bga_driver *)device->driver;

    write_register(BGA_COMMAND_ENABLE, 0x00);
    write_register(BGA_COMMAND_XRES, driver->ivideo.xres);
    write_register(BGA_COMMAND_YRES, driver->ivideo.yres);
    write_register(BGA_COMMAND_BPP, driver->ivideo.bpp);
    write_register(BGA_COMMAND_ENABLE, 0x40 | 0x01);

}

static unsigned int read_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)device->driver;
    unsigned int size = driver->ivideo.xres * driver->ivideo.yres * driver->ivideo.bpp / 4;

    return memory_read(buffer, count, driver->lfb, size, offset);

}

static unsigned int write_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct bga_driver *driver = (struct bga_driver *)device->driver;
    unsigned int size = driver->ivideo.xres * driver->ivideo.yres * driver->ivideo.bpp / 4;

    return memory_write(driver->lfb, size, buffer, count, offset);

}

void bga_init_driver(struct bga_driver *driver)
{

    memory_clear(driver, sizeof (struct bga_driver));
    base_init_driver(&driver->base, "bga", check, attach);
    base_video_init_interface(&driver->ivideo, mode, read_data, write_data, 0, 0);

    driver->ivideo.xres = 800;
    driver->ivideo.yres = 600;
    driver->ivideo.bpp = BGA_BPP_32;
    driver->bank = (void *)0xA0000;

}

