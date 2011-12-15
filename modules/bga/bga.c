#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/bga/bga.h>

static struct bga_driver driver;

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_INDEX, index);
    io_outw(BGA_DATA, data);

}

static unsigned short read_register(unsigned short index)
{

    io_outw(BGA_INDEX, index);

    return io_inw(BGA_DATA);

}

static void bga_driver_set_mode(struct bga_driver *self, unsigned int xres, unsigned int yres, unsigned int bpp)
{

    self->xres = xres;
    self->yres = yres;
    self->bpp = bpp;

    write_register(BGA_INDEX_ENABLE, 0x00);
    write_register(BGA_INDEX_XRES, self->xres);
    write_register(BGA_INDEX_YRES, self->yres);
    write_register(BGA_INDEX_BPP, self->bpp);
    write_register(BGA_INDEX_ENABLE, 0x01 | 0x40);

}

static void bga_driver_set_bank(struct bga_driver *self, unsigned int index)
{

    write_register(BGA_INDEX_BANK, index);

}

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    //struct pci_device *device = (struct pci_device *)driver.base.device;
    //unsigned int *video = device->configuration.bar0;

    unsigned int *video = (unsigned int *)driver.bank;

    unsigned int offset = (y * driver.xres + x);

    *(video + offset) = color;

}

static void draw_example()
{

    unsigned int i;
    unsigned int j;

    // Red square
    for (i = 10; i < 20; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(i, j, 0x00FF0000);

    // Green square
    for (i = 20; i < 30; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(i, j, 0x0000FF00);

    // Blue square
    for (i = 30; i < 40; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(i, j, 0x000000FF);

}

void bga_driver_init(struct bga_driver *driver, struct pci_device *device)
{

    modules_driver_init(&driver->base, BGA_DRIVER_TYPE);
    driver->base.device = &device->base;
    driver->bank = (unsigned int *)0xA0000;
    driver->set_mode = bga_driver_set_mode;
    driver->set_bank = bga_driver_set_bank;

    driver->set_mode(driver, 1024, 768, BGA_BPP_32);
    driver->set_bank(driver, 0);

    draw_example();

}

void init()
{

    struct pci_bus *bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE);

    if (!bus)
        return;

    struct pci_device *device = bus->find_device_by_id(bus, 0x1234, 0x1111);

    if (!device)
        return;

    bga_driver_init(&driver, device);

    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

