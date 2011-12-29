#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus busses[8];
static struct pci_device devices[64];
static unsigned int devicesCount;

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

static void add_device(struct pci_bus *bus, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &devices[devicesCount];

    pci_device_init(device, bus, slot, function, get_address(bus->num, slot, function));
    modules_register_device(&device->base);

    devicesCount++;

}

static unsigned int add_bus(struct pci_bus *bus, unsigned int num)
{

    pci_bus_init(bus, num);

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(bus->num, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            return pci_inb(address, 0x19);

        if ((header & 0x02))
            return pci_inb(address, 0x18);

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(bus->num, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    continue;

                add_device(bus, slot, function);

            }

            continue;

        }
    
        add_device(bus, slot, 0);

    }

    return 0;

}

void init()
{

    devicesCount = 0;

    unsigned int i = 0;
    unsigned int num = 0;

    for (i = 0; (num = add_bus(&busses[i], num)); i++)
        modules_register_bus(&busses[i].base);

}

void destroy()
{

}

