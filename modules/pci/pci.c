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

    unsigned int i;

    for (i = 0; i < pciDevicesCount; i++)
    {

        if (&pciDevices[i].base.node == node)
            return &pciDevices[i];

    }

    return 0;

}

static unsigned int pci_device_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    struct pci_device *device = pci_get_device(node);

    if (!device)
        return 0;

    char num[32];

    string_copy(buffer, "Vendor: 0x");
    string_copy_num(num, device->configuration.vendor, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Device: 0x");
    string_copy_num(num, device->configuration.device, 16);
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

static unsigned short pci_read(unsigned short bus, unsigned short slot, unsigned short func, unsigned short offset)
{

    unsigned int lbus = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned int tmp = 0;

    unsigned int address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    io_outd(PCI_ADDRESS, address);

    tmp = (unsigned short)((io_ind(PCI_DATA) >> ((offset & 2) * 8)) & 0xFFFF);

    return tmp;

}

static void pci_add(unsigned short bus, unsigned short slot, unsigned short function)
{

    struct pci_device *device = &pciDevices[pciDevicesCount];

    string_copy(device->base.name, "pci:");
    string_copy_num(device->base.name + 4, bus, 10);
    string_concat(device->base.name, ":");
    string_copy_num(device->base.name + 6, slot, 10);
    string_concat(device->base.name, ":");
    string_copy_num(device->base.name + 8, function, 10);

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = MODULES_DEVICE_TYPE_PCI;
    device->base.node.operations.read = pci_device_read;
    device->configuration.vendor = pci_read(bus, slot, function, 0x00);
    device->configuration.device = pci_read(bus, slot, function, 0x02);
    device->configuration.revision = (pci_read(bus, slot, function, 0x08) & 0xFF);
    device->configuration.interface = (pci_read(bus, slot, function, 0x08) >> 8);
    device->configuration.subclass = (pci_read(bus, slot, function, 0x0A) & 0xFF);
    device->configuration.classcode = (pci_read(bus, slot, function, 0x0A) >> 8);
    device->configuration.headertype = (pci_read(bus, slot, function, 0x0E) & 0xFF);

    if (device->configuration.headertype == 0x00)
    {

        device->configuration.bar0 = pci_read(bus, slot, function, 0x10);
        device->configuration.bar0 |= pci_read(bus, slot, function, 0x12) << 16;
        device->configuration.bar1 = pci_read(bus, slot, function, 0x14);
        device->configuration.bar1 |= pci_read(bus, slot, function, 0x16) << 16;
        device->configuration.bar2 = pci_read(bus, slot, function, 0x18);
        device->configuration.bar2 |= pci_read(bus, slot, function, 0x1A) << 16;
        device->configuration.bar3 = pci_read(bus, slot, function, 0x1C);
        device->configuration.bar3 |= pci_read(bus, slot, function, 0x1E) << 16;
        device->configuration.bar4 = pci_read(bus, slot, function, 0x20);
        device->configuration.bar4 |= pci_read(bus, slot, function, 0x22) << 16;
        device->configuration.bar5 = pci_read(bus, slot, function, 0x24);
        device->configuration.bar5 |= pci_read(bus, slot, function, 0x26) << 16;

    }

    modules_register_device(&device->base);

    pciDevicesCount++;

}

static void pci_scan_bus(unsigned short bus)
{

    unsigned short slot;

    for (slot = 0; slot < 32; slot++)
    {

        if (pci_read(bus, slot, 0, 0) == 0xFFFF)
            continue;

        unsigned short header = pci_read(bus, slot, 0, 0xE);

        if ((header & 0x01))
            pci_scan_bus(pci_read(bus, slot, 0, 0x18) >> 8);

        if ((header & 0x02))
            pci_scan_bus(pci_read(bus, slot, 0, 0x18) & 0xFF);

        if ((header & 0x80))
        {

            unsigned int function;

            for (function = 0; function < 8; function++)
            {

                if ((pci_read(bus, slot, function, 0)) == 0xFFFF)
                    break;

                pci_add(bus, slot, function);

            }

        }
    
        else
        {

            pci_add(bus, slot, 0);

        }

    }

}

static void pci_init_busses()
{

    pciBus.base.module.type = MODULES_TYPE_BUS;
    pciBus.base.type = MODULES_BUS_TYPE_PCI;
    string_copy(pciBus.base.name, "pci:0");
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

