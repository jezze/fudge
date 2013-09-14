#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pci.h"

unsigned int pci_device_ind(struct pci_device *device, unsigned short offset)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    return pci_bus_ind(bus, device->num, device->slot, device->function, offset);

}

unsigned short pci_device_inw(struct pci_device *device, unsigned short offset)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    return pci_bus_inw(bus, device->num, device->slot, device->function, offset);

}

unsigned char pci_device_inb(struct pci_device *device, unsigned short offset)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    return pci_bus_inb(bus, device->num, device->slot, device->function, offset);

}

void pci_device_outd(struct pci_device *device, unsigned short offset, unsigned int value)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    pci_bus_outd(bus, device->num, device->slot, device->function, offset, value);

}

void pci_device_outw(struct pci_device *device, unsigned short offset, unsigned short value)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    pci_bus_outw(bus, device->num, device->slot, device->function, offset, value);

}

void pci_device_outb(struct pci_device *device, unsigned short offset, unsigned char value)
{

    struct pci_bus *bus = (struct pci_bus *)device->base.bus;

    pci_bus_outb(bus, device->num, device->slot, device->function, offset, value);

}

void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int irq, unsigned int num, unsigned int slot, unsigned int function)
{

    memory_clear(device, sizeof (struct pci_device));
    base_init_device(&device->base, PCI_DEVICE_TYPE, irq, "pci:0000:0000", &bus->base);

    device->num = num;
    device->slot = slot;
    device->function = function;

}

