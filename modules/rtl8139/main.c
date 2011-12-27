#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/vfs.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

static void handle_irq()
{

}

static unsigned int check_device(struct modules_module *module)
{

    if (!modules_is_device(module))
        return 0;

    if (((struct modules_device *)module)->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *device = (struct pci_device *)module;

    return device->configuration.vendorid == 0x10EC && device->configuration.deviceid == 0x8139;

}

static void attach_device(struct modules_module *module)
{

    struct pci_device *device = (struct pci_device *)module;
    device->base.driver = (struct modules_driver *)&driver;

    driver.io = (device->configuration.bar0 & ~1);

    kernel_register_irq(device->configuration.interruptline, handle_irq);

}

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);

    modules_foreach(check_device, attach_device);

    driver.start(&driver);

}

void destroy()
{

}

