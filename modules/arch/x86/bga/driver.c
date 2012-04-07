#include <lib/io.h>
#include <lib/memory.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <modules/bga/bga.h>
#include <modules/pci/pci.h>

static struct mmu_memory memory;

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_INDEX, index);
    io_outw(BGA_DATA, data);

}

static void set_mode(struct bga_driver *self)
{

    write_register(BGA_INDEX_ENABLE, 0x00);
    write_register(BGA_INDEX_XRES, self->xres);
    write_register(BGA_INDEX_YRES, self->yres);
    write_register(BGA_INDEX_BPP, self->bpp);
    write_register(BGA_INDEX_ENABLE, 0x40 | 0x01);

}

static void start(struct modules_driver *self)
{

    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_memory_init(&memory, (unsigned int)driver->lfb, (unsigned int)driver->lfb, 0x400000); 
    mmu_map_kernel_memory(&memory);
    mmu_reload_memory();

}

static void attach(struct modules_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;
    struct bga_driver *driver = (struct bga_driver *)device->driver;

    driver->lfb = (void *)pciDevice->config_ind(pciDevice, PCI_CONFIG_BAR0);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *pciDevice = (struct pci_device *)device;

    return pciDevice->config_inw(pciDevice, PCI_CONFIG_VENDOR) == 0x1234 && pciDevice->config_inw(pciDevice, PCI_CONFIG_DEVICE) == 0x1111;

}

void bga_driver_init(struct bga_driver *driver)
{

    memory_clear(driver, sizeof (struct bga_driver));

    modules_driver_init(&driver->base, BGA_DRIVER_TYPE, "bga", start, check, attach);

    driver->xres = 800;
    driver->yres = 600;
    driver->bpp = BGA_BPP_32;
    driver->bank = (void *)0xA0000;
    driver->set_mode = set_mode;

}

