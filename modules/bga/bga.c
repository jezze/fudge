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

static void set_mode(unsigned int width, unsigned int height, unsigned int depth, unsigned int linear, unsigned int clear)
{

    write_register(BGA_INDEX_ENABLE, 0x00);
    write_register(BGA_INDEX_XRES, width);
    write_register(BGA_INDEX_YRES, height);
    write_register(BGA_INDEX_YRES, height);
    write_register(BGA_INDEX_BPP, depth);
    write_register(BGA_INDEX_ENABLE, 0x40 | 0x01);

}

static void set_bank(unsigned int index)
{

    write_register(BGA_INDEX_BANK, index);

}

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int *video = (unsigned int *)0xA0000;

    unsigned int offset = (y * 320 + x) * 1;

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

    set_mode(320, 240, BGA_BPP_32, 0, 0);
    set_bank(0);

    draw_example();

}

void init()
{

    struct pci_bus *bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE);

    if (!bus)
        return;

    struct pci_device *device = bus->find_device(bus, 0x1111);

    if (!device)
        return;

    bga_driver_init(&driver, device);

    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

