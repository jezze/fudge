#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

void rtl8139_init()
{

    struct pci_bus *bus = (struct pci_bus *)modules_get_bus(MODULES_BUS_TYPE_PCI);

    if (!bus)
        return;

    struct pci_device *device = bus->find_device(0x8139);

    if (!device)
        return;

}

