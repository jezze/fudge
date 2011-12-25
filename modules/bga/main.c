#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/bga/bga.h>

static struct bga_driver driver;

static unsigned int check_device(struct modules_module *module)
{

    if (!pci_device_check_module(module))
        return 0;

    struct pci_device *device = (struct pci_device *)module;

    return device->configuration.vendorid == 0x1234 && device->configuration.deviceid == 0x1111;

}

static void init_driver(struct modules_module *module)
{

    struct pci_device *device = (struct pci_device *)module;

    bga_driver_init(&driver, device);
    modules_register_driver(&driver.base);

}

void init()
{

    modules_foreach(check_device, init_driver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

