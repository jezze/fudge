#include <module.h>
#include <kernel/resource.h>
#include <x86/arch/mmu.h>
#include <system/system.h>
#include <base/base.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>

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

static struct base_driver driver;
static struct base_video_interface ivideo;
static struct base_device device;
static struct base_video_node node;
static void *bank;
static void *lfb;

static void write_register(unsigned short index, unsigned short data)
{

    io_outw(BGA_REGISTER_COMMAND, index);
    io_outw(BGA_REGISTER_DATA, data);

}

static void set_mode(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp)
{

    write_register(BGA_COMMAND_ENABLE, 0x00);
    write_register(BGA_COMMAND_XRES, xres);
    write_register(BGA_COMMAND_YRES, yres);
    write_register(BGA_COMMAND_BPP, bpp);
    write_register(BGA_COMMAND_ENABLE, 0x40 | 0x01);

}

static unsigned int read_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

/*
    unsigned int size = ivideo.xres * ivideo.yres * ivideo.bpp / 4;

    return memory_read(buffer, count, lfb, size, offset);
*/

    return 0;

}

static unsigned int write_data(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

/*
    unsigned int size = ivideo.xres * ivideo.yres * ivideo.bpp / 4;

    return memory_write(lfb, size, buffer, count, offset);
*/

    return 0;

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inw(bus, id, PCI_CONFIG_VENDOR) == BGA_PCI_VENDOR && pci_inw(bus, id, PCI_CONFIG_DEVICE) == BGA_PCI_DEVICE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    base_init_device(&device, bus, id);
    base_video_init_node(&node, &device, &ivideo);
    base_video_register_node(&node);

    bank = (void *)0xA0000;
    lfb = (void *)(unsigned long)pci_ind(bus, id, PCI_CONFIG_BAR0);

/*
    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_map_kernel_memory(3, (unsigned int)driver->lfb, (unsigned int)driver->lfb, 0x00400000);
    mmu_reload_memory();
*/

}

static void detach(struct base_bus *bus, unsigned int id)
{

    base_video_unregister_node(&node);

}

void init()
{

    base_video_init_interface(&ivideo, set_mode, read_data, write_data, 0, 0);
    base_video_register_interface(&ivideo);
    base_init_driver(&driver, "bga", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_video_unregister_interface(&ivideo);
    base_unregister_driver(&driver);

}

