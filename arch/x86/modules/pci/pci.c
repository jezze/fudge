#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/pci/pci.h>

static struct modules_bus pciBus;
static struct pci_device pciDevices[256];
static unsigned int pciDevicesCount;

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

    if ((vendor == pci_read(bus, slot, 0, 0)) == 0xFFFF)
        return;

    struct pci_device *device = &pciDevices[pciDevicesCount];

    string_copy(device->base.name, "pci:0:0");
    device->base.node.length = 0;
    device->vendor = vendor;
    device->device = pci_read(bus, slot, 0, 2);

    modules_register_device(MODULES_DEVICE_TYPE_PCI, &device->base);

    pciDevicesCount++;

}

void pci_init()
{

    string_copy(pciBus.name, "pci");
    modules_register_bus(MODULES_BUS_TYPE_PCI, &pciBus);

    pci_check_vendor(0, 0);
    pci_check_vendor(0, 1);
    pci_check_vendor(0, 2);
    pci_check_vendor(0, 3);
    pci_check_vendor(0, 4);
    pci_check_vendor(0, 5);
    pci_check_vendor(0, 6);
    pci_check_vendor(0, 7);

}

