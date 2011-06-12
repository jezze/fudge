#include <lib/file.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/pci/pci.h>

unsigned short pci_read(unsigned short bus, unsigned short slot, unsigned short func, unsigned short offset)
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

unsigned short pci_check_vendor(unsigned short bus, unsigned short slot)
{

    unsigned short vendor;
    unsigned short device;

    if ((vendor == pci_read(bus, slot, 0, 0)) != 0xFFFF)
    {

        device = pci_read(bus, slot, 0, 2);

    }

    return vendor;

}

void pci_init()
{

}

