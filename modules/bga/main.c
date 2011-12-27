#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/bga/bga.h>

static struct bga_driver driver;

static unsigned int check_device(struct modules_module *module)
{

    if (!modules_is_device(module))
        return 0;

    if (((struct modules_device *)module)->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *device = (struct pci_device *)module;

    return device->configuration.vendorid == 0x1234 && device->configuration.deviceid == 0x1111;

}

static void idriver(struct modules_module *module)
{

    struct pci_device *device = (struct pci_device *)module;

    bga_driver_init(&driver, device);
    modules_register_driver(&driver.base);

}

void init()
{

    modules_foreach(check_device, idriver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

