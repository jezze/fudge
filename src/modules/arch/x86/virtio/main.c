#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define PCIVENDOR                       0x1AF4
#define TYPENETWORK                     1
#define TYPEBLOCK                       2
#define TYPETERMINAL                    3
#define TYPEENTROPY                     4
#define TYPEBALLOON                     5
#define TYPEIOMEM                       6
#define TYPERPMSG                       7
#define TYPESCSI                        8
#define TYPE9P                          9
#define TYPEWLAN                        10
#define REGISTERDEVFEATURES             0x00
#define REGISTERGUESTFEATURES           0x04
#define REGISTERQADDRESS                0x08
#define REGISTERQSIZE                   0x0C
#define REGISTERQSELECT                 0x0E
#define REGISTERQNOTIFY                 0x10
#define REGISTERSTATUS                  0x12
#define REGISTERISR                     0x13

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static unsigned short io;

static void reset(void)
{

    io_outb(io + REGISTERSTATUS, 0);
    io_inb(io + REGISTERSTATUS);

}

static void handleirq(unsigned int irq)
{

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    return 0;

}

static void driver_init(void)
{

    ethernet_initinterface(&ethernetinterface, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR;

}

static void driver_attach(unsigned int id)
{

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;

    pci_setmaster(id);
    reset();
    ethernet_registerinterface(&ethernetinterface, id);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(pci_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "virtio", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

