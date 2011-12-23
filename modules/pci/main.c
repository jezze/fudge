#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus busses[8];

unsigned int get_address(unsigned int num, unsigned int slot, unsigned int function)
{

    return 0x80000000 | (num << 16) | (slot << 11) | (function << 8);

}

static void pci_bus_add_device(struct pci_bus *bus, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &bus->devices[bus->devicesCount];

    pci_device_init(device, bus, slot, function, get_address(bus->num, slot, function));

    modules_register_device(&device->base);

    bus->devicesCount++;

}

static unsigned int scan(struct pci_bus *bus, unsigned int num)
{

    pci_bus_init(bus, num);

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(bus->num, slot, 0x00);

        if (pci_bus_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_bus_inb(address, 0x0E);

        if ((header & 0x01))
            return pci_bus_inb(address, 0x19);

        if ((header & 0x02))
            return pci_bus_inb(address, 0x18);

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(bus->num, slot, function);

                if (pci_bus_inw(address, 0x00) == 0xFFFF)
                    continue;

                pci_bus_add_device(bus, slot, function);

            }

            continue;

        }
    
        pci_bus_add_device(bus, slot, 0);

    }

    return 0;

}

void init()
{

    unsigned int i = 0;
    unsigned int num = 0;

    for (i = 0; (num = scan(&busses[i], num)); i++);
        modules_register_bus(&busses[i].base);

}

void destroy()
{

}

