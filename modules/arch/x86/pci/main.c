#include <fudge.h>
#include <kernel.h>
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

unsigned int pci_ind(unsigned int id, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA);

}

unsigned short pci_inw(unsigned int id, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 2) * 8);

}

unsigned char pci_inb(unsigned int id, unsigned short offset)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));

    return io_ind(PCI_REGISTER_DATA) >> ((offset & 3) * 8);

}

void pci_outd(unsigned int id, unsigned short offset, unsigned int value)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));
    io_outd(PCI_REGISTER_DATA, value);

}

void pci_outw(unsigned int id, unsigned short offset, unsigned short value)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));
    io_outw(PCI_REGISTER_DATA, value);

}

void pci_outb(unsigned int id, unsigned short offset, unsigned char value)
{

    io_outd(PCI_REGISTER_CONTROL, id | (offset & 0xFC));
    io_outb(PCI_REGISTER_DATA, value);

}

void pci_setmaster(unsigned int id)
{

    unsigned short command = pci_inw(id, PCI_CONFIG_COMMAND);

    pci_outw(id, PCI_CONFIG_COMMAND, command | (1 << 2));

}

unsigned short pci_getirq(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_LINE);

}

static void add(unsigned int address)
{

    devices.address[devices.count] = address;
    devices.count++;

}

static unsigned int calcaddress(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

static void detect(unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int header;

        if (pci_inw(calcaddress(num, slot, 0), 0x00) == 0xFFFF)
            continue;

        header = pci_inb(calcaddress(num, slot, 0), 0x0E);

        if ((header & 0x01))
            detect(pci_inb(calcaddress(num, slot, 0), 0x19));

        if ((header & 0x02))
            detect(pci_inb(calcaddress(num, slot, 0), 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                if (pci_inw(calcaddress(num, slot, function), 0x00) == 0xFFFF)
                    continue;

                add(calcaddress(num, slot, function));

            }

            continue;

        }

        add(calcaddress(num, slot, 0));

    }

}

static void bus_setup()
{

    detect(0);

}

static unsigned int bus_next(unsigned int id)
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

void module_init()
{

    devices.count = 0;
    memory_clear(devices.address, sizeof (unsigned int) * 64);

    base_initbus(&bus, PCI_BUS_TYPE, "pci", bus_setup, bus_next);

}

void module_register()
{

    base_registerbus(&bus);

}

void module_unregister()
{

    base_unregisterbus(&bus);

}

