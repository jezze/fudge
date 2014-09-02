#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "pci.h"

enum pci_register
{

    PCI_REGISTER_CONTROL                = 0x0CF8,
    PCI_REGISTER_DATA                   = 0x0CFC

};

static struct base_bus bus;
static struct {unsigned int address[64]; unsigned int count;} devices;

unsigned int pci_ind(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA);

}

unsigned short pci_inw(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 2) * 8);

}

unsigned char pci_inb(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 3) * 8);

}

void pci_outd(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned int value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outd(PCI_REGISTER_DATA, value);

}

void pci_outw(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned short value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outw(PCI_REGISTER_DATA, value);

}

void pci_outb(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned char value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outb(PCI_REGISTER_DATA, value);

}

static void add(struct base_bus *bus, unsigned int address)
{

    devices.address[devices.count] = address;
    devices.count++;

}

static unsigned int calcaddress(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

static void detect(struct base_bus *bus, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;

        if (pci_inw(bus, calcaddress(num, slot, 0), 0x00) == 0xFFFF)
            continue;

        header = pci_inb(bus, calcaddress(num, slot, 0), 0x0E);

        if ((header & 0x01))
            detect(bus, pci_inb(bus, calcaddress(num, slot, 0), 0x19));

        if ((header & 0x02))
            detect(bus, pci_inb(bus, calcaddress(num, slot, 0), 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                if (pci_inw(bus, calcaddress(num, slot, function), 0x00) == 0xFFFF)
                    continue;

                add(bus, calcaddress(num, slot, function));

            }

            continue;

        }

        add(bus, calcaddress(num, slot, 0));

    }

}

static void bus_setup(struct base_bus *self)
{

    detect(self, 0);

}

static unsigned int bus_next(struct base_bus *self, unsigned int id)
{

    unsigned int i;

    if (!id)
        return devices.address[0];

    for (i = 0; i < devices.count; i++)
    {

        if (devices.address[i] == id)
            return devices.address[i + 1];

    }

    return 0;

}

static unsigned short bus_irq(struct base_bus *self, unsigned int id)
{

    return pci_inb(self, id, PCI_CONFIG_LINE);

}

void init()
{

    devices.count = 0;
    memory_clear(devices.address, sizeof (unsigned int) * 64);

    base_init_bus(&bus, PCI_BUS_TYPE, "pci", bus_setup, bus_next, bus_irq);
    base_register_bus(&bus);

}

void destroy()
{

    base_unregister_bus(&bus);

}

