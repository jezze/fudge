#include <base/base.h>
#include <arch/x86/pci/pci.h>

static struct pci_bus bus;

void init()
{

    pci_init_bus(&bus);
    base_register_bus(&bus.base);

}

void destroy()
{

    base_unregister_bus(&bus.base);

}

