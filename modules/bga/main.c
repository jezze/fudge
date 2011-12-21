#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/bga/bga.h>

static struct bga_driver driver;

void init()
{

    unsigned int i;
    struct pci_bus *bus;

    for (i = 0; (bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE, i)); i++)
    {

        struct pci_device *device = bus->find_device_by_id(bus, 0x1234, 0x1111, 0);

        if (device)
        {

            bga_driver_init(&driver, device);
            modules_register_driver(&driver.base);

            break;

        }

    }

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

