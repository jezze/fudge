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
static struct {struct pci_device item[64]; unsigned int count;} devices;

static unsigned int calculate_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

unsigned int pci_bus_ind(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA);

}

unsigned short pci_bus_inw(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 2) * 8);

}

unsigned char pci_bus_inb(struct base_bus *bus, unsigned int address, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 3) * 8);

}

void pci_bus_outd(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned int value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outd(PCI_REGISTER_DATA, value);

}

void pci_bus_outw(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned short value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outw(PCI_REGISTER_DATA, value);

}

void pci_bus_outb(struct base_bus *bus, unsigned int address, unsigned short offset, unsigned char value)
{

    io_outd(PCI_REGISTER_CONTROL, address | (offset & 0xFC));
    io_outb(PCI_REGISTER_DATA, value);

}

static void add_device(struct base_bus *bus, unsigned int address)
{

    struct pci_device *device = &devices.item[devices.count];

    base_init_device(&device->base, PCI_DEVICE_TYPE, "pci", bus);
    base_register_device(&device->base);

    device->address = address;

    devices.count++;

}

static void detect(struct base_bus *bus, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;

        if (pci_bus_inw(bus, calculate_address(num, slot, 0), 0x00) == 0xFFFF)
            continue;

        header = pci_bus_inb(bus, calculate_address(num, slot, 0), 0x0E);

        if ((header & 0x01))
            detect(bus, pci_bus_inb(bus, calculate_address(num, slot, 0), 0x19));

        if ((header & 0x02))
            detect(bus, pci_bus_inb(bus, calculate_address(num, slot, 0), 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                if (pci_bus_inw(bus, calculate_address(num, slot, function), 0x00) == 0xFFFF)
                    continue;

                add_device(bus, calculate_address(num, slot, function));

            }

            continue;

        }

        add_device(bus, calculate_address(num, slot, 0));

    }

}

static void setup(struct base_bus *self)
{

    detect(self, 0);

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    return pci_bus_inb(self, id, PCI_CONFIG_LINE);

}

void init()
{

    base_init_bus(&bus, "pci", setup, device_irq);
    base_register_bus(&bus);

}

void destroy()
{

    base_unregister_bus(&bus);

}

