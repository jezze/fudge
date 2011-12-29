#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

unsigned int get_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

unsigned int pci_ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

unsigned short pci_inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((pci_ind(address, offset) >> ((offset & 2) * 8)) & 0xFFFF);

}

unsigned char pci_inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((pci_ind(address, offset) >> ((offset & 3) * 8)) & 0xFF);

}

static unsigned int pci_bus_scan(struct pci_bus *self, unsigned int num, void (*callback)(unsigned int num, unsigned int slot, unsigned int function))
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(num, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            pci_bus_scan(self, pci_inb(address, 0x19), callback);

        if ((header & 0x02))
            pci_bus_scan(self, pci_inb(address, 0x18), callback);

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(num, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    continue;

                callback(num, slot, function);

            }

            continue;

        }

        callback(num, slot, 0);

    }

    return 0;

}

void pci_bus_init(struct pci_bus *bus)
{

    modules_bus_init(&bus->base, PCI_BUS_TYPE);
    bus->scan = pci_bus_scan;

}

