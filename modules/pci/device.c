#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

void pci_device_init(struct pci_device *device, struct pci_bus *bus, unsigned int slot, unsigned int function, unsigned int address)
{

    modules_device_init(&device->base, PCI_DEVICE_TYPE);
    device->bus = bus;
    device->slot = slot;
    device->function = function;
    device->configuration.vendorid = pci_inw(address, 0x00);
    device->configuration.deviceid = pci_inw(address, 0x02);
    device->configuration.revision = pci_inb(address, 0x08);
    device->configuration.interface = pci_inb(address, 0x09);
    device->configuration.subclasscode = pci_inb(address, 0x0A);
    device->configuration.classcode = pci_inb(address, 0x0B);
    device->configuration.headertype = pci_inb(address, 0x0E);
    device->configuration.interruptline = pci_inb(address, 0x3C);
    device->configuration.interruptpin = pci_inb(address, 0x3D);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = pci_ind(address, 0x10);
        device->configuration.bar1 = pci_ind(address, 0x14);
        device->configuration.bar2 = pci_ind(address, 0x18);
        device->configuration.bar3 = pci_ind(address, 0x1C);
        device->configuration.bar4 = pci_ind(address, 0x20);
        device->configuration.bar5 = pci_ind(address, 0x24);

    }

}

