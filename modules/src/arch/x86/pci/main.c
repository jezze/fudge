#include <base/base.h>
#include <arch/x86/pci/pci.h>

static struct pci_bus bus;

void init()
{

    pci_bus_init(&bus);
    base_register_bus(&bus.base);

}

void destroy()
{

    base_unregister_bus(&bus.base);

}

