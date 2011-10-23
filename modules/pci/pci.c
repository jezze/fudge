#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>

static struct pci_bus pciBus;
static struct pci_device pciDevices[32];
static unsigned int pciDevicesCount;

static struct pci_device *pci_get_device(struct vfs_node *node)
{
/*
    unsigned int i;

    for (i = 0; i < pciDevicesCount; i++)
    {

        if (&pciDevices[i].base.node == node)
            return &pciDevices[i];

    }
*/
    return 0;

}

static unsigned int pci_device_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct pci_device *device = pci_get_device(self);

    if (!device)
        return 0;

    char num[32];

    string_copy(buffer, "Vendor: 0x");
    string_copy_num(num, device->configuration.vendorid, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Device: 0x");
    string_copy_num(num, device->configuration.deviceid, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Class: 0x");
    string_copy_num(num, device->configuration.classcode, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Sublass: 0x");
    string_copy_num(num, device->configuration.subclass, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Interface: 0x");
    string_copy_num(num, device->configuration.interface, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Headertype: 0x");
    string_copy_num(num, device->configuration.headertype, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Interrupt line: 0x");
    string_copy_num(num, device->configuration.interruptline, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Interrupt pin: 0x");
    string_copy_num(num, device->configuration.interruptpin, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    if (device->configuration.headertype == 0x00)
    {

        string_concat(buffer, "Bar0: ");
        string_copy_num(num, device->configuration.bar0, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

        string_concat(buffer, "Bar1: ");
        string_copy_num(num, device->configuration.bar1, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

        string_concat(buffer, "Bar2: ");
        string_copy_num(num, device->configuration.bar2, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

        string_concat(buffer, "Bar3: ");
        string_copy_num(num, device->configuration.bar3, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

        string_concat(buffer, "Bar4: ");
        string_copy_num(num, device->configuration.bar4, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

        string_concat(buffer, "Bar5: ");
        string_copy_num(num, device->configuration.bar5, 2);
        string_concat(buffer, num);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

static unsigned int pci_get_address(unsigned int bus, unsigned int slot, unsigned int function)
{

    return (0x80000000 | (bus << 16) | (slot << 11) | (function << 8));

}

static unsigned int pci_ind(unsigned int address, unsigned short offset)
{

    io_outd(PCI_ADDRESS, address | (offset & 0xFC));

    return io_ind(PCI_DATA);

}

static unsigned char pci_inb(unsigned int address, unsigned short offset)
{

    return (unsigned char)((pci_ind(address, offset) >> ((offset & 3) * 8)) & 0xFF);

}

static unsigned short pci_inw(unsigned int address, unsigned short offset)
{

    return (unsigned short)((pci_ind(address, offset) >> ((offset & 2) * 8)) & 0xFFFF);

}

static void pci_add(unsigned short bus, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &pciDevices[pciDevicesCount];

    unsigned int address = pci_get_address(bus, slot, function);

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = PCI_DEVICE_TYPE;
    device->configuration.vendorid = pci_inw(address, 0x00);
    device->configuration.deviceid = pci_inw(address, 0x02);
    device->configuration.revision = pci_inb(address, 0x08);
    device->configuration.interface = pci_inb(address, 0x09);
    device->configuration.subclass = pci_inb(address, 0x0A);
    device->configuration.classcode = pci_inb(address, 0x0B);
    device->configuration.headertype = pci_inb(address, 0x0E);
    device->configuration.interruptline = pci_inb(address, 0x3C);
    device->configuration.interruptpin = pci_inb(address, 0x3D);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = pci_ind(address, 0x10);
        device->configuration.bar1 = pci_ind(address, 0x14);
        device->configuration.bar2 = pci_ind(address, 0x18);
        device->configuration.bar3 = pci_ind(address, 0x1C);
        device->configuration.bar4 = pci_ind(address, 0x20);
        device->configuration.bar5 = pci_ind(address, 0x24);

    }

    modules_register_device(&device->base);

    pciDevicesCount++;

}

static void pci_scan_bus(unsigned int bus)
{

    unsigned int slot;

    for (slot = 0; slot < 32; slot++)
    {

        unsigned int address = pci_get_address(bus, slot, 0x00);

        if (pci_inw(address, 0x00) == 0xFFFF)
            continue;

        unsigned short header = pci_inb(address, 0x0E);

        if ((header & 0x01))
            pci_scan_bus(pci_inb(address, 0x19));

        if ((header & 0x02))
            pci_scan_bus(pci_inb(address, 0x18));

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                unsigned int address = pci_get_address(bus, slot, function);

                if (pci_inw(address, 0x00) == 0xFFFF)
                    break;

                pci_add(bus, slot, function);

            }

            continue;

        }
    
        pci_add(bus, slot, 0);

    }

}

struct pci_device *pci_bus_find_device(unsigned short deviceid)
{

    unsigned int i;

    for (i = 0; i < pciDevicesCount; i++)
    {

        if (pciDevices[i].configuration.deviceid == deviceid)
            return &pciDevices[i];

    }

    return 0;

}

static void pci_init_busses()
{

    pciBus.base.module.type = MODULES_TYPE_BUS;
    pciBus.base.type = PCI_BUS_TYPE;
    pciBus.find_device = pci_bus_find_device;

    modules_register_bus(&pciBus.base);

}

static void pci_init_devices()
{

    pciDevicesCount = 0;

    pci_scan_bus(0);

}

void pci_init()
{

    pci_init_busses();
    pci_init_devices();

}

