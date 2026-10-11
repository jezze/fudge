#include <fudge.h>
#include <kernel.h>
#include <modules/bus.h>
#include <modules/driver.h>
#include <modules/io/io.h>
#include "pci.h"

#define REG_CONTROL                     0x0CF8
#define REG_DATA                        0x0CFC

static struct base_bus bus;
static struct {unsigned int address[64]; unsigned int count;} devices;

unsigned int pci_ind(unsigned int id, unsigned short offset)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));

    return io_ind(REG_DATA);

}

unsigned short pci_inw(unsigned int id, unsigned short offset)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));

    return io_ind(REG_DATA) >> ((offset & 2) * 8);

}

unsigned char pci_inb(unsigned int id, unsigned short offset)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));

    return io_ind(REG_DATA) >> ((offset & 3) * 8);

}

void pci_outd(unsigned int id, unsigned short offset, unsigned int value)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));
    io_outd(REG_DATA, value);

}

void pci_outw(unsigned int id, unsigned short offset, unsigned short value)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));
    io_outw(REG_DATA + (offset & 2), value);

}

void pci_outb(unsigned int id, unsigned short offset, unsigned char value)
{

    io_outd(REG_CONTROL, id | (offset & 0xFC));
    io_outb(REG_DATA + (offset & 3), value);

}

void pci_setmaster(unsigned int id)
{

    unsigned short command = pci_inw(id, PCI_CONFIG_COMMAND);

    pci_outw(id, PCI_CONFIG_COMMAND, command | PCI_COMMAND_MASTER);

}

unsigned short pci_getirq(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_LINE);

}

unsigned char pci_findcapability(unsigned int id, unsigned char capability)
{

    unsigned char offset;
    unsigned int i;

    if (!(pci_inw(id, PCI_CONFIG_STATUS) & PCI_STATUS_CAPABILITIES))
        return 0;

    offset = pci_inb(id, PCI_CONFIG_CAPABILITIES) & 0xFC;

    for (i = 0; offset && i < 48; i++)
    {

        if (pci_inb(id, offset) == capability)
            return offset;

        offset = pci_inb(id, offset + 1) & 0xFC;

    }

    return 0;

}

unsigned int pci_setmsi(unsigned int id, unsigned int address, unsigned int data)
{

    unsigned char offset = pci_findcapability(id, PCI_CAPABILITY_MSI);
    unsigned short control;

    if (!offset)
        return 0;

    control = pci_inw(id, offset + 2);

    pci_outd(id, offset + 4, address);

    if (control & PCI_MSI_64BIT)
    {

        pci_outd(id, offset + 8, 0);
        pci_outw(id, offset + 12, data);

    }

    else
    {

        pci_outw(id, offset + 8, data);

    }

    pci_outw(id, offset + 2, (control & ~PCI_MSI_MULTIPLE) | PCI_MSI_ENABLE);
    pci_outw(id, PCI_CONFIG_COMMAND, pci_inw(id, PCI_CONFIG_COMMAND) | PCI_COMMAND_INTXDISABLE);

    return 1;

}

void pci_unsetmsi(unsigned int id)
{

    unsigned char offset = pci_findcapability(id, PCI_CAPABILITY_MSI);

    if (!offset)
        return;

    pci_outw(id, offset + 2, pci_inw(id, offset + 2) & ~PCI_MSI_ENABLE);
    pci_outw(id, PCI_CONFIG_COMMAND, pci_inw(id, PCI_CONFIG_COMMAND) & ~PCI_COMMAND_INTXDISABLE);

}

static unsigned int calcaddress(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

static void add(unsigned int address)
{

    devices.address[devices.count] = address;
    devices.count++;

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

static void bus_setup(void)
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

/*
static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    unsigned int cnum;
    unsigned int cslot;
    unsigned int cfunction;

    for (cnum = 0; cnum < 256; cnum++)
    {

        for (cslot = 0; cslot < 32; cslot++)
        {

            unsigned int x = calcaddress(cnum, cslot, 0x00);
            unsigned char header;

            if (x <= id)
                continue;

            if (pci_inw(x, 0x00) == 0xFFFF)
                continue;

            header = pci_inb(x, 0x0E);

            if (header & 0x80)
            {

                for (cfunction = 0; cfunction < 8; cfunction++)
                {

                    unsigned int y = calcaddress(cnum, cslot, cfunction);

                    if (y <= id)
                        continue;

                    if (pci_inw(y, 0x00) == 0xFFFF)
                        continue;

                    return y;

                }

            }

            return x;

        }

    }

    return 0;

}
*/

void module_init(void)
{

    base_initbus(&bus, PCI_BUS, "pci", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}

