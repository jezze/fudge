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

static void attach_device(struct modules_module *module)
{

    struct pci_device *device = (struct pci_device *)module;
    device->base.driver = (struct modules_driver *)&driver;

}

void init()
{

    bga_driver_init(&driver);
    modules_register_driver(&driver.base);

    modules_foreach(check_device, attach_device);

    driver.start(&driver);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

