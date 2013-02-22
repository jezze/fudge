#include <fudge/module.h>
#include <base/base.h>
#include "pci.h"

static struct pci_bus bus;

unsigned int pci_calculate_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

void init()
{

    unsigned int i;

    pci_init_bus(&bus);
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

