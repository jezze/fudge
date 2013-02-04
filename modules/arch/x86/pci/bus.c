#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "pci.h"

static unsigned int get_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

unsigned int pci_bus_ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

unsigned short pci_bus_inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((pci_bus_ind(address, offset) >> ((offset & 2) * 8)));

}

unsigned char pci_bus_inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((pci_bus_ind(address, offset) >> ((offset & 3) * 8)));

}

static void add_device(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned int address)
{

    struct pci_device *device = &bus->devices[bus->devicesCount];

    pci_init_device(device, bus, num, slot, function, address);
    base_register_device(&device->base);

    bus->devicesCount++;

}

static void detect(struct pci_bus *bus, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;
        unsigned int address = get_address(num, slot, 0x00);

        if (pci_bus_inw(address, 0x00) == 0xFFFF)
            continue;

        header = pci_bus_inb(address, 0x0E);

        if ((header & 0x01))
            detect(bus, pci_bus_inb(address, 0x19));

        if ((header & 0x02))
            detect(bus, pci_bus_inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(num, slot, function);

                if (pci_bus_inw(address, 0x00) == 0xFFFF)
                    continue;

                add_device(bus, num, slot, function, address);

            }

            continue;

        }

        add_device(bus, num, slot, 0, address);

    }

}

static void scan(struct base_bus *self)
{

    struct pci_bus *bus = (struct pci_bus *)self;

    detect(bus, 0);

}

void pci_init_bus(struct pci_bus *bus)
{

    memory_clear(bus, sizeof (struct pci_bus));
    base_init_bus(&bus->base, PCI_BUS_TYPE, "pci", scan);

}

