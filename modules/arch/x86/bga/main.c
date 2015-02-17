#include <fudge.h>
#include <kernel.h>
#include <arch/x86/mmu.h>
#include <system/system.h>
#include <base/base.h>
#include <video/video.h>
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
static struct video_interface videointerface;
static void *bank;
static void *lfb;

static void setreg(unsigned short index, unsigned short data)
{

    io_outw(BGA_REGISTER_COMMAND, index);
    io_outw(BGA_REGISTER_DATA, data);

}

static void videointerface_setmode(unsigned int xres, unsigned int yres, unsigned int bpp)
{

    setreg(BGA_COMMAND_ENABLE, 0x00);
    setreg(BGA_COMMAND_XRES, xres);
    setreg(BGA_COMMAND_YRES, yres);
    setreg(BGA_COMMAND_BPP, bpp);
    setreg(BGA_COMMAND_ENABLE, 0x40 | 0x01);

}

static unsigned int videointerface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

/*
    unsigned int size = videointerface.xres * videointerface.yres * videointerface.bpp / 4;

    return memory_read(buffer, count, lfb, size, offset);
*/

    return 0;

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

/*
    unsigned int size = videointerface.xres * videointerface.yres * videointerface.bpp / 4;

    return memory_write(lfb, size, buffer, count, offset);
*/

    return 0;

}

static void driver_init(struct base_driver *self)
{

    video_initinterface(&videointerface, self, videointerface_setmode, videointerface_rdata, videointerface_wdata, 0, 0);

}

static unsigned int driver_match(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inw(bus, id, PCI_CONFIG_VENDOR) == BGA_PCI_VENDOR && pci_inw(bus, id, PCI_CONFIG_DEVICE) == BGA_PCI_DEVICE;

}

static void driver_attach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    bank = (void *)0xA0000;
    lfb = (void *)(unsigned long)pci_ind(bus, id, PCI_CONFIG_BAR0);

/*
    struct bga_driver *driver = (struct bga_driver *)self;

    mmu_map_kernel_memory(3, (unsigned int)driver->lfb, (unsigned int)driver->lfb, 0x00400000);
    mmu_reload_memory();
*/

    video_registerinterface(&videointerface, bus, id);

}

static void driver_detach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init()
{

    base_initdriver(&driver, "bga", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

