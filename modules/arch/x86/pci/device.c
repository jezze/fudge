#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "pci.h"

void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int address)
{

    memory_clear(device, sizeof (struct pci_device));
    base_init_device(&device->base, PCI_DEVICE_TYPE, "pci", &bus->base);

    device->address = address;

}

