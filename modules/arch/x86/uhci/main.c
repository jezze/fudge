#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <arch/x86/pci/pci.h>

enum uhci_register
{

    UHCI_REGISTER_COMMAND               = 0x00,
    UHCI_REGISTER_STATUS                = 0x02,
    UHCI_REGISTER_INTERRUPT             = 0x04,
    UHCI_REGISTER_FRAMEINDEX            = 0x06,
    UHCI_REGISTER_FRAMEBASE             = 0x08,
    UHCI_REGISTER_FRAMEMOD              = 0x0C,
    UHCI_REGISTER_PORT1                 = 0x10,
    UHCI_REGISTER_PORT2                 = 0x12

};

static struct base_driver driver;

static void driver_init()
{

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(bus, id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    /*
    unsigned int bar0 = pci_ind(bus, id, PCI_CONFIG_BAR4);
    */

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

}

void module_init()
{

    base_initdriver(&driver, "uhci", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

