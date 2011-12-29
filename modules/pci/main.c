#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_bus bus;
static struct pci_device devices[64];
static unsigned int devicesCount;

static void add_device(unsigned int num, unsigned int slot, unsigned int function)
{

    struct pci_device *device = &devices[devicesCount];

    pci_device_init(device, &bus, num, slot, function, 0x80000000 | (num << 16) | (slot << 11) | (function << 8));
    modules_register_device(&device->base);

    devicesCount++;

}

void init()
{

    devicesCount = 0;

    pci_bus_init(&bus);
    modules_register_bus(&bus.base);

    bus.scan(&bus, 0, add_device);

}

void destroy()
{

    modules_unregister_bus(&bus.base);

}

