#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus bus;

static unsigned int get_address(unsigned int busX, unsigned int slot, unsigned int function)
{

    return (0x80000000 | (busX << 16) | (slot << 11) | (function << 8));

}

static unsigned int ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

static unsigned char inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((ind(address, offset) >> ((offset & 3) * 8)) & 0xFF);

}

static unsigned short inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((ind(address, offset) >> ((offset & 2) * 8)) & 0xFFFF);

}

void pci_device_init(struct pci_device *device, unsigned int address)
{

    modules_device_init(&device->base, PCI_DEVICE_TYPE);
    device->configuration.vendorid = inw(address, 0x00);
    device->configuration.deviceid = inw(address, 0x02);
    device->configuration.revision = inb(address, 0x08);
    device->configuration.interface = inb(address, 0x09);
    device->configuration.subclass = inb(address, 0x0A);
    device->configuration.classcode = inb(address, 0x0B);
    device->configuration.headertype = inb(address, 0x0E);
    device->configuration.interruptline = inb(address, 0x3C);
    device->configuration.interruptpin = inb(address, 0x3D);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = ind(address, 0x10);
        device->configuration.bar1 = ind(address, 0x14);
        device->configuration.bar2 = ind(address, 0x18);
        device->configuration.bar3 = ind(address, 0x1C);
        device->configuration.bar4 = ind(address, 0x20);
        device->configuration.bar5 = ind(address, 0x24);

    }

    log_write("[pci] %x:%x\n", device->configuration.vendorid, device->configuration.deviceid);

}

static void pci_bus_add_device(unsigned short busX, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &bus.devices[bus.devicesCount];

    pci_device_init(device, get_address(busX, slot, function));

    modules_register_device(&device->base);

    bus.devicesCount++;

}

static struct pci_device *pci_bus_find_device(struct pci_bus *self, unsigned int vendorid, unsigned short deviceid)
{

    unsigned int i;

    for (i = 0; i < self->devicesCount; i++)
    {

        if (self->devices[i].configuration.vendorid == vendorid && self->devices[i].configuration.deviceid == deviceid)
            return &self->devices[i];

    }

    return 0;

}

static void pci_bus_scan(unsigned int busX)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = get_address(busX, slot, 0x00);

        if (inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = inb(address, 0x0E);

        if ((header & 0x01))
            pci_bus_scan(inb(address, 0x19));

        if ((header & 0x02))
            pci_bus_scan(inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = get_address(busX, slot, function);

                if (inw(address, 0x00) == 0xFFFF)
                    continue;

                pci_bus_add_device(busX, slot, function);

            }

            continue;

        }
    
        pci_bus_add_device(busX, slot, 0);

    }

}

void pci_bus_init(struct pci_bus *bus)
{

    modules_bus_init(&bus->base, PCI_BUS_TYPE);
    bus->devicesCount = 0;
    bus->find_device = pci_bus_find_device;
    bus->scan = pci_bus_scan;
    bus->scan(0);

}

void init()
{

    log_write("[pci] Init\n");

    pci_bus_init(&bus);

    modules_register_bus(&bus.base);

}

void destroy()
{

}

