#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <arch/x86/io/io.h>
#include "pci.h"

#define REGISTERCONTROL                 0x0CF8
#define REGISTERDATA                    0x0CFC

static struct base_bus bus;

unsigned int pci_ind(unsigned int id, unsigned short offset)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));

    return io_ind(REGISTERDATA);

}

unsigned short pci_inw(unsigned int id, unsigned short offset)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));

    return io_ind(REGISTERDATA) >> ((offset & 2) * 8);

}

unsigned char pci_inb(unsigned int id, unsigned short offset)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));

    return io_ind(REGISTERDATA) >> ((offset & 3) * 8);

}

void pci_outd(unsigned int id, unsigned short offset, unsigned int value)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));
    io_outd(REGISTERDATA, value);

}

void pci_outw(unsigned int id, unsigned short offset, unsigned short value)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));
    io_outw(REGISTERDATA, value);

}

void pci_outb(unsigned int id, unsigned short offset, unsigned char value)
{

    io_outd(REGISTERCONTROL, id | (offset & 0xFC));
    io_outb(REGISTERDATA, value);

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

static unsigned int calcaddress(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

static void bus_setup()
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

void module_init()
{

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

