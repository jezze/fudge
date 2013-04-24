#include <fudge/module.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "pci.h"

unsigned int pci_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

unsigned int pci_bus_ind(struct pci_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(bus->control, address | (offset & 0xFC));

    return io_ind(bus->data);

}

unsigned short pci_bus_inw(struct pci_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(bus->control, address | (offset & 0xFC));

    return (unsigned short)(io_ind(bus->data) >> ((offset & 2) * 8));

}

unsigned char pci_bus_inb(struct pci_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(bus->control, address | (offset & 0xFC));

    return (unsigned char)(io_ind(bus->data) >> ((offset & 3) * 8));

}

static void add_device(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function)
{

    struct pci_device *device = &bus->devices.item[bus->devices.count];

    pci_init_device(device, bus, num, slot, function);

    bus->devices.count++;

}

static void detect(struct pci_bus *bus, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;
        unsigned int address = pci_address(num, slot, 0);

        if (pci_bus_inw(bus, address, 0x00) == 0xFFFF)
            continue;

        header = pci_bus_inb(bus, address, 0x0E);

        if ((header & 0x01))
            detect(bus, pci_bus_inb(bus, address, 0x19));

        if ((header & 0x02))
            detect(bus, pci_bus_inb(bus, address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = pci_address(num, slot, function);

                if (pci_bus_inw(bus, address, 0x00) == 0xFFFF)
                    continue;

                add_device(bus, num, slot, function);

            }

            continue;

        }

        add_device(bus, num, slot, 0);

    }

}

static void scan(struct base_bus *self)
{

    struct pci_bus *bus = (struct pci_bus *)self;

    detect(bus, 0);

}

void pci_init_bus(struct pci_bus *bus, unsigned short control, unsigned short data)
{

    memory_clear(bus, sizeof (struct pci_bus));
    base_init_bus(&bus->base, PCI_BUS_TYPE, "pci", scan);

    bus->control = control;
    bus->data = data;

}

