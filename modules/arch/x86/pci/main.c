#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pci.h"

enum pci_register
{

    PCI_REGISTER_CONTROL                = 0x0CF8,
    PCI_REGISTER_DATA                   = 0x0CFC

};

static struct pci_bus bus;

void init()
{

    unsigned int i;

    pci_init_bus(&bus, PCI_REGISTER_CONTROL, PCI_REGISTER_DATA);
    base_register_bus(&bus.base);

    for (i = 0; i < bus.devices.count; i++)
        base_register_device(&bus.devices.item[i].base);

}

void destroy()
{

    unsigned int i;

    for (i = 0; i < bus.devices.count; i++)
        base_unregister_device(&bus.devices.item[i].base);

    base_unregister_bus(&bus.base);

}

