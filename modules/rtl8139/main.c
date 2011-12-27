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

//    if (!pci_device_check_module(module))
//        return 0;

    struct pci_device *device = (struct pci_device *)module;

    return device->configuration.vendorid == 0x10EC && device->configuration.deviceid == 0x8139;

}

static void idriver(struct modules_module *module)
{

    struct pci_device *device = (struct pci_device *)module;

    rtl8139_driver_init(&driver, device);
    kernel_register_irq(device->configuration.interruptline, handle_irq);
    modules_register_driver(&driver.base);

}

void init()
{

    modules_foreach(check_device, idriver);

}

void destroy()
{

    struct pci_device *device = (struct pci_device *)driver.base.device;

    kernel_unregister_irq(device->configuration.interruptline);
    modules_unregister_driver(&driver.base);

}

