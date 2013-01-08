#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "pci.h"

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

