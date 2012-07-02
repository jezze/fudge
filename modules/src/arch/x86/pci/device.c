#include <io.h>
#include <memory.h>
#include <modules.h>
#include <arch/x86/pci/pci.h>

static unsigned int config_ind(struct pci_device *device, unsigned int offset)
{

    return device->bus->ind(device->address, offset);

}

static unsigned short config_inw(struct pci_device *device, unsigned int offset)
{

    return device->bus->inw(device->address, offset);

}

static unsigned char config_inb(struct pci_device *device, unsigned int offset)
{

    return device->bus->inb(device->address, offset);

}

void pci_device_init(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned int address)
{

    memory_clear(device, sizeof (struct pci_device));

    modules_device_init(&device->base, PCI_DEVICE_TYPE, "pci");

    device->bus = bus;
    device->num = num;
    device->slot = slot;
    device->function = function;
    device->address = address;
    device->config_ind = config_ind;
    device->config_inw = config_inw;
    device->config_inb = config_inb;

}

