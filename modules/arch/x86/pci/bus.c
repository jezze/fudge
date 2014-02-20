#include <module.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "pci.h"

static unsigned int calculate_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

unsigned int pci_bus_ind(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));

    return io_ind(bus->data);

}

unsigned short pci_bus_inw(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));

    return io_ind(bus->data) >> ((offset & 2) * 8);

}

unsigned char pci_bus_inb(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));

    return io_ind(bus->data) >> ((offset & 3) * 8);

}

void pci_bus_outd(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned int value)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));
    io_outd(bus->data, value);

}

void pci_bus_outw(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned short value)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));
    io_outw(bus->data, value);

}

void pci_bus_outb(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned char value)
{

    io_outd(bus->control, calculate_address(num, slot, function) | (offset & 0xFC));
    io_outb(bus->data, value);

}

static void add_device(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function)
{

    struct pci_device *device = &bus->devices.item[bus->devices.count];
    unsigned int irq = pci_bus_inb(bus, num, slot, function, PCI_CONFIG_LINE);

    pci_init_device(device, bus, irq, num, slot, function);

    bus->devices.count++;

}

static void detect(struct pci_bus *bus, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;

        if (pci_bus_inw(bus, num, slot, 0, 0x00) == 0xFFFF)
            continue;

        header = pci_bus_inb(bus, num, slot, 0, 0x0E);

        if ((header & 0x01))
            detect(bus, pci_bus_inb(bus, num, slot, 0, 0x19));

        if ((header & 0x02))
            detect(bus, pci_bus_inb(bus, num, slot, 0, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                if (pci_bus_inw(bus, num, slot, function, 0x00) == 0xFFFF)
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

