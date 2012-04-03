#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
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

static unsigned int scan(struct pci_bus *self, unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(num, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            scan(self, pci_inb(address, 0x19));

        if ((header & 0x02))
            scan(self, pci_inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(num, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    continue;

                self->add_device(self, num, slot, function);

            }

            continue;

        }

        self->add_device(self, num, slot, 0);

    }

    return 0;

}

static void add_device(struct pci_bus *self, unsigned int num, unsigned int slot, unsigned int function)
{

    struct pci_device *device = &self->devices[self->devicesCount];

    pci_device_init(device, self, num, slot, function, 0x80000000 | (num << 16) | (slot << 11) | (function << 8));
    modules_register_device(&device->base);

    self->devicesCount++;

}

void pci_bus_init(struct pci_bus *bus)
{

    memory_clear(bus, sizeof (struct pci_bus));

    modules_bus_init(&bus->base, PCI_BUS_TYPE, "pci:0");

    bus->scan = scan;
    bus->add_device = add_device;

}

