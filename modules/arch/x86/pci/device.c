#include <lib/memory.h>
#include <kernel/modules.h>
#include <kernel/arch/x86/io.h>
#include <modules/pci/pci.h>

void pci_device_init(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned int address)
{

    memory_clear(device, sizeof (struct pci_device));

    modules_device_init(&device->base, PCI_DEVICE_TYPE, "pci:0:0");

    device->bus = bus;
    device->num = num;
    device->slot = slot;
    device->function = function;
    device->configuration.vendorid = bus->inw(address, 0x00);
    device->configuration.deviceid = bus->inw(address, 0x02);
    device->configuration.revision = bus->inb(address, 0x08);
    device->configuration.interface = bus->inb(address, 0x09);
    device->configuration.subclasscode = bus->inb(address, 0x0A);
    device->configuration.classcode = bus->inb(address, 0x0B);
    device->configuration.headertype = bus->inb(address, 0x0E);
    device->configuration.interruptline = bus->inb(address, 0x3C);
    device->configuration.interruptpin = bus->inb(address, 0x3D);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = bus->ind(address, 0x10);
        device->configuration.bar1 = bus->ind(address, 0x14);
        device->configuration.bar2 = bus->ind(address, 0x18);
        device->configuration.bar3 = bus->ind(address, 0x1C);
        device->configuration.bar4 = bus->ind(address, 0x20);
        device->configuration.bar5 = bus->ind(address, 0x24);

    }

}

