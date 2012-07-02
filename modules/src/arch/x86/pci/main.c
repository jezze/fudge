#include <modules.h>
#include <arch/x86/pci/pci.h>

static struct pci_bus bus;

void init()
{

    pci_bus_init(&bus);
    modules_register_bus(&bus.base);

}

void destroy()
{

    modules_unregister_bus(&bus.base);

}

