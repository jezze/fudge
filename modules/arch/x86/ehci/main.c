#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <arch/x86/pci/pci.h>

enum ehci_register
{

    EHCI_REGISTER_COMMAND               = 0x00,
    EHCI_REGISTER_STATUS                = 0x04,
    EHCI_REGISTER_INTERRUPT             = 0x08,
    EHCI_REGISTER_FRAMEINDEX            = 0x0C,
    EHCI_REGISTER_SEGMENT               = 0x10,
    EHCI_REGISTER_LISTBASE              = 0x14,
    EHCI_REGISTER_LISTNEXT              = 0x18,
    EHCI_REGISTER_FLAG                  = 0x40

};

static struct base_driver driver;

static unsigned int driver_check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(bus, id, PCI_CONFIG_INTERFACE) == 0x20;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    /*
    unsigned int bar0 = pci_ind(bus, id, PCI_CONFIG_BAR0);
    */

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

}

void init()
{

    base_init_driver(&driver, "ehci", driver_check, driver_attach, driver_detach);
    base_register_driver(&driver);

}

void destroy()
{

    base_unregister_driver(&driver);

}

