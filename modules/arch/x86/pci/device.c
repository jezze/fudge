#include <fudge/module.h>
#include <base/base.h>
#include "pci.h"

unsigned int pci_device_ind(struct pci_device *device, unsigned short offset)
{

    return pci_ind(pci_address(device->num, device->slot, device->function), offset);

}

unsigned short pci_device_inw(struct pci_device *device, unsigned short offset)
{

    return pci_inw(pci_address(device->num, device->slot, device->function), offset);

}

unsigned char pci_device_inb(struct pci_device *device, unsigned short offset)
{

    return pci_inb(pci_address(device->num, device->slot, device->function), offset);

}

void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function)
{

    memory_clear(device, sizeof (struct pci_device));
    base_init_device(&device->base, PCI_DEVICE_TYPE, 0, "pci");

    device->bus = bus;
    device->num = num;
    device->slot = slot;
    device->function = function;

}

