#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

unsigned int pci_bus_ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

unsigned char pci_bus_inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((pci_bus_ind(address, offset) >> ((offset & 3) * 8)) & 0xFF);

}

unsigned short pci_bus_inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((pci_bus_ind(address, offset) >> ((offset & 2) * 8)) & 0xFFFF);

}

void pci_device_init(struct pci_device *device, struct pci_bus *bus, unsigned int slot, unsigned int function, unsigned int address)
{

    modules_device_init(&device->base, PCI_DEVICE_TYPE);
    device->bus = bus;
    device->slot = slot;
    device->function = function;
    device->configuration.vendorid = pci_bus_inw(address, 0x00);
    device->configuration.deviceid = pci_bus_inw(address, 0x02);
    device->configuration.revision = pci_bus_inb(address, 0x08);
    device->configuration.interface = pci_bus_inb(address, 0x09);
    device->configuration.subclasscode = pci_bus_inb(address, 0x0A);
    device->configuration.classcode = pci_bus_inb(address, 0x0B);
    device->configuration.headertype = pci_bus_inb(address, 0x0E);
    device->configuration.interruptline = pci_bus_inb(address, 0x3C);
    device->configuration.interruptpin = pci_bus_inb(address, 0x3D);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = pci_bus_ind(address, 0x10);
        device->configuration.bar1 = pci_bus_ind(address, 0x14);
        device->configuration.bar2 = pci_bus_ind(address, 0x18);
        device->configuration.bar3 = pci_bus_ind(address, 0x1C);
        device->configuration.bar4 = pci_bus_ind(address, 0x20);
        device->configuration.bar5 = pci_bus_ind(address, 0x24);

    }

    log_write("[pci] %x:%x %x:%x\n", device->configuration.vendorid, device->configuration.deviceid, device->configuration.classcode, device->configuration.subclasscode);

}

