#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>

#define VIRTIO_PCI_VENDOR               0x1AF4

enum
{

    VIRTIO_TYPE_NETWORK                 = 1,
    VIRTIO_TYPE_BLOCK                   = 2,
    VIRTIO_TYPE_TERMINAL                = 3,
    VIRTIO_TYPE_ENTROPY                 = 4,
    VIRTIO_TYPE_BALLOON                 = 5,
    VIRTIO_TYPE_IOMEM                   = 6,
    VIRTIO_TYPE_RPMSG                   = 7,
    VIRTIO_TYPE_SCSI                    = 8,
    VIRTIO_TYPE_9P                      = 9,
    VIRTIO_TYPE_WLAN                    = 10

};

enum
{

    VIRTIO_REGISTER_DEVFEATURES         = 0x00,
    VIRTIO_REGISTER_GUESTFEATURES       = 0x04,
    VIRTIO_REGISTER_QADDRESS            = 0x08,
    VIRTIO_REGISTER_QSIZE               = 0x0C,
    VIRTIO_REGISTER_QSELECT             = 0x0E,
    VIRTIO_REGISTER_QNOTIFY             = 0x10,
    VIRTIO_REGISTER_STATUS              = 0x12,
    VIRTIO_REGISTER_ISR                 = 0x13

};

static struct base_driver driver;
static unsigned short io;

static void reset()
{

    io_outb(io + VIRTIO_REGISTER_STATUS, 0); 
    io_inb(io + VIRTIO_REGISTER_STATUS); 

}

static void handleirq(unsigned int irq, unsigned int id)
{

}

static void driver_init()
{

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    if (type != PCI_BUS_TYPE)
        return 0;

    return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_PCI_VENDOR;

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;

    memory_copy((void *)0xB8000, "a ", 2);
    pci_setmaster(id);
    reset();

    pic_setroutine(irq, id, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    pic_unsetroutine(irq, id);

}

void module_init()
{

    base_initdriver(&driver, "virtio", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

