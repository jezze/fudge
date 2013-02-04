#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pci.h"

void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned int address)
{

    memory_clear(device, sizeof (struct pci_device));
    base_init_device(&device->base, PCI_DEVICE_TYPE, 0, "pci");

    device->bus = bus;
    device->num = num;
    device->slot = slot;
    device->function = function;
    device->address = address;

}

