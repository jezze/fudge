#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

static void handle_irq()
{

}

void init()
{

    unsigned int i;
    struct pci_bus *bus;

    for (i = 0; (bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE, i)); i++)
    {

        struct pci_device *device = bus->find_device_by_id(bus, 0x10EC, 0x8139, 0);

        if (device)
        {

            rtl8139_driver_init(&driver, device);
            kernel_register_irq(device->configuration.interruptline, handle_irq);
            modules_register_driver(&driver.base);

            break;

        }

    }

}

void destroy()
{

    struct pci_device *device = (struct pci_device *)driver.base.device;

    kernel_unregister_irq(device->configuration.interruptline);
    modules_unregister_driver(&driver.base);

}

