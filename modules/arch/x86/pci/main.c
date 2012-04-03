#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus bus;

void init()
{

    pci_bus_init(&bus);

    modules_register_bus(&bus.base);

    bus.scan(&bus, 0);

}

void destroy()
{

    modules_unregister_bus(&bus.base);

}

