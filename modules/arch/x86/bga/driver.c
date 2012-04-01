#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/bga/bga.h>

static struct mmu_memory memory;

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_INDEX, index);
    io_outw(BGA_DATA, data);

}

static void draw_pixel(struct bga_driver *self, unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int *video = (unsigned int *)self->lfb;

    unsigned int offset = (y * self->xres + x);

    *(video + offset) = color;

}

static void draw_example(struct bga_driver *self)
{

    unsigned int i;
    unsigned int j;

    // Background
    for (i = 0; i < 800; i++)
        for (j = 0; j < 600; j++)
            draw_pixel(self, i, j, 0x00334455);

    // Red square
    for (i = 10; i < 20; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(self, i, j, 0x00FF0000);

    // Green square
    for (i = 20; i < 30; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(self, i, j, 0x0000FF00);

    // Blue square
    for (i = 30; i < 40; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(self, i, j, 0x000000FF);

}

static void set_mode(struct bga_driver *self, unsigned int xres, unsigned int yres, unsigned int bpp)
{

    self->xres = xres;
    self->yres = yres;
    self->bpp = bpp;

    write_register(BGA_INDEX_ENABLE, 0x00);
    write_register(BGA_INDEX_XRES, self->xres);
    write_register(BGA_INDEX_YRES, self->yres);
    write_register(BGA_INDEX_BPP, self->bpp);
    write_register(BGA_INDEX_ENABLE, 0x40 | 0x01);

}

static void start(struct modules_driver *self)
{

    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_memory_init(&memory, driver->lfb, driver->lfb, 0x400000); 
    mmu_map_kernel_memory(&memory);
    mmu_reload_memory();

    driver->set_mode(driver, 800, 600, BGA_BPP_32);
    draw_example(driver);

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct bga_driver *driver = (struct bga_driver *)self;
    struct pci_device *pciDevice = (struct pci_device *)device;

    driver->lfb = (unsigned int *)pciDevice->configuration.bar0;

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *pciDevice = (struct pci_device *)device;

    return pciDevice->configuration.vendorid == 0x1234 && pciDevice->configuration.deviceid == 0x1111;

}

void bga_driver_init(struct bga_driver *driver)
{

    memory_clear(driver, sizeof (struct bga_driver));

    modules_driver_init(&driver->base, BGA_DRIVER_TYPE, "bga");

    driver->base.start = start;
    driver->base.attach = attach;
    driver->base.check = check;
    driver->bank = (unsigned int *)0xA0000;
    driver->set_mode = set_mode;

}

