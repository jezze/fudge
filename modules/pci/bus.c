#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

void pci_bus_init(struct pci_bus *bus)
{

    modules_bus_init(&bus->base, PCI_BUS_TYPE);

}

