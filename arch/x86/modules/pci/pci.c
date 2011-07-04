#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/pci/pci.h>

static struct modules_bus pciBus;
static struct pci_device pciDevices[16];
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

static unsigned int pci_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
    {

        out[0] = '0';

        return 1;

    }

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    string_copy(out, buffer + i + 1);

    return 32 - i;

}

static unsigned int pci_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct pci_device *device = pci_get_device(node);

    if (!device)
        return 0;

    char num[32];

    string_copy(buffer, "Vendor: 0x");
    pci_write_num(num, device->vendor, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Device: 0x");
    pci_write_num(num, device->device, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Class: 0x");
    pci_write_num(num, device->classcode, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Sublass: 0x");
    pci_write_num(num, device->subclass, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

    string_concat(buffer, "Interface: 0x");
    pci_write_num(num, device->interface, 16);
    string_concat(buffer, num);
    string_concat(buffer, "\n");

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

static void pci_check_vendor(unsigned short bus, unsigned short slot)
{

    unsigned short vendor;

    if ((vendor = pci_read(bus, slot, 0, 0)) == 0xFFFF)
        return;

    struct pci_device *device = &pciDevices[pciDevicesCount];

    string_copy(device->base.name, "pci:");
    pci_write_num(device->base.name + 4, bus, 10);
    string_concat(device->base.name, ":");
    pci_write_num(device->base.name + 6, slot, 10);

    device->base.node.length = 0;
    device->base.node.operations.read = pci_device_read;
    device->vendor = vendor;
    device->device = pci_read(bus, slot, 0, 2);
    device->interface = (pci_read(bus, slot, 0, 8) >> 8);
    device->revision = (pci_read(bus, slot, 0, 8) & 0xFF);
    device->classcode = (pci_read(bus, slot, 0, 10) >> 8);
    device->subclass = (pci_read(bus, slot, 0, 10) & 0xFF);

    modules_register_device(MODULES_DEVICE_TYPE_PCI, &device->base);

    pciDevicesCount++;

}

static void pci_init_busses()
{

    string_copy(pciBus.name, "pci:0");
    modules_register_bus(MODULES_BUS_TYPE_PCI, &pciBus);

}

static void pci_init_devices()
{

    pciDevicesCount = 0;

    pci_check_vendor(0, 0);
    pci_check_vendor(0, 1);
    pci_check_vendor(0, 2);
    pci_check_vendor(0, 3);
    pci_check_vendor(0, 4);
    pci_check_vendor(0, 5);
    pci_check_vendor(0, 6);
    pci_check_vendor(0, 7);

}

void pci_init()
{

    pci_init_busses();
    pci_init_devices();

}

