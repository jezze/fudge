#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pci/pci.h>

#define REGISTERCOMMAND                 0x00
#define REGISTERSTATUS                  0x02
#define REGISTERINTERRUPT               0x04
#define REGISTERFRAMEINDEX              0x06
#define REGISTERFRAMEBASE               0x08
#define REGISTERFRAMEMOD                0x0C
#define REGISTERPORT1                   0x10
#define REGISTERPORT2                   0x12

static struct base_driver driver;

static void driver_init(void)
{

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_attach(unsigned int id)
{

    /*
    unsigned int bar0 = pci_ind(id, PCI_CONFIG_BAR4);
    */

}

static void driver_detach(unsigned int id)
{

}

void module_init(void)
{

    base_initdriver(&driver, "uhci", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

