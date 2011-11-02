#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>

static struct pci_bus pciBus;

static unsigned int pci_ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

static unsigned char pci_inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((pci_ind(address, offset) >> ((offset & 3) * 8)) & 0xFF);

}

static unsigned short pci_inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((pci_ind(address, offset) >> ((offset & 2) * 8)) & 0xFFFF);

}

void pci_device_init(struct pci_device *device, unsigned int address)
{

    modules_device_init(&device->base, PCI_DEVICE_TYPE);
    device->configuration.vendorid = pci_inw(address, 0x00);
    device->configuration.deviceid = pci_inw(address, 0x02);
    device->configuration.revision = pci_inb(address, 0x08);
    device->configuration.interface = pci_inb(address, 0x09);
    device->configuration.subclass = pci_inb(address, 0x0A);
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

static unsigned int pci_get_address(unsigned int bus, unsigned int slot, unsigned int function)
{

    return (0x80000000 | (bus << 16) | (slot << 11) | (function << 8));

}

static void pci_bus_add(unsigned short bus, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &pciBus.devices[pciBus.devicesCount];

    pci_device_init(device, pci_get_address(bus, slot, function));

    modules_register_device(&device->base);

    pciBus.devicesCount++;

}

static struct pci_device *pci_bus_find_device(struct pci_bus *self, unsigned short deviceid)
{

    unsigned int i;

    for (i = 0; i < self->devicesCount; i++)
    {

        if (self->devices[i].configuration.deviceid == deviceid)
            return &self->devices[i];

    }

    return 0;

}

static void pci_bus_scan(unsigned int bus)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = pci_get_address(bus, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            pci_bus_scan(pci_inb(address, 0x19));

        if ((header & 0x02))
            pci_bus_scan(pci_inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = pci_get_address(bus, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    break;

                pci_bus_add(bus, slot, function);

            }

            continue;

        }
    
        pci_bus_add(bus, slot, 0);

    }

}

void pci_bus_init(struct pci_bus *bus)
{

    modules_bus_init(&bus->base, PCI_BUS_TYPE);
    bus->devicesCount = 0;
    bus->find_device = pci_bus_find_device;
    bus->scan = pci_bus_scan;
    bus->scan(0);

}

void pci_init()
{

    pci_bus_init(&pciBus);

    modules_register_bus(&pciBus.base);

}

