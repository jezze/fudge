#include <fudge/module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "cirrus.h"

#define CIRRUS_PCI_VENDOR               0x1013
#define CIRRUS_PCI_DEVICE               0x00B8

static void mode(struct base_device *device)
{

}

static unsigned int read_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int read_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int read_video_info(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static void attach(struct base_device *device)
{

    struct cirrus_driver *driver = (struct cirrus_driver *)device->driver;

    base_video_register_interface(&driver->ivideo, device);

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inw(pciDevice, PCI_CONFIG_VENDOR) == CIRRUS_PCI_VENDOR && pci_device_inw(pciDevice, PCI_CONFIG_DEVICE) == CIRRUS_PCI_DEVICE;

}

void cirrus_init_driver(struct cirrus_driver *driver)
{

    memory_clear(driver, sizeof (struct cirrus_driver));
    base_init_driver(&driver->base, "cirrus", check, attach);
    base_video_init_interface(&driver->ivideo, mode, read_video_data, write_video_data, read_video_colormap, write_video_colormap, read_video_info);

    driver->ivideo.xres = 80;
    driver->ivideo.yres = 25;
    driver->ivideo.bpp = 16;

}

