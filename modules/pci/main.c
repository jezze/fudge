#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus bus;
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

static void add_device(unsigned int num, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &devices[devicesCount];

    pci_device_init(device, &bus, num, slot, function, get_address(num, slot, function));
    modules_register_device(&device->base);

    devicesCount++;

}

static unsigned int scan(unsigned int num)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(num, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            scan(pci_inb(address, 0x19));

        if ((header & 0x02))
            scan(pci_inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(num, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    continue;

                add_device(num, slot, function);

            }

            continue;

        }
    
        add_device(num, slot, 0);

    }

    return 0;

}

void init()
{

    devicesCount = 0;

    pci_bus_init(&bus);
    modules_register_bus(&bus.base);

    scan(0);

}

void destroy()
{

    modules_unregister_bus(&bus.base);

}

