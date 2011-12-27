#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>

static struct pci_device *pci_bus_find_device_by_id(struct pci_bus *self, unsigned short vendorid, unsigned short deviceid, unsigned int index)
{

    unsigned int i;

    for (i = 0; i < self->devicesCount; i++)
    {

        if (self->devices[i].configuration.vendorid != vendorid || self->devices[i].configuration.deviceid != deviceid)
            continue;

        if (index)
        {

            index--;

            continue;

        }

        return &self->devices[i];

    }

    return 0;

}

static struct pci_device *pci_bus_find_device_by_class(struct pci_bus *self, unsigned char classcode, unsigned char subclasscode, unsigned int index)
{

    unsigned int i;

    for (i = 0; i < self->devicesCount; i++)
    {

        if (self->devices[i].configuration.classcode != classcode || self->devices[i].configuration.subclasscode != subclasscode)
            continue;

        if (index)
        {

            index--;

            continue;

        }

        return &self->devices[i];

    }

    return 0;

}

void pci_bus_init(struct pci_bus *bus, unsigned int num)
{

    modules_bus_init(&bus->base, PCI_BUS_TYPE);
    bus->num = num;
    bus->devicesCount = 0;
    bus->find_device_by_id = pci_bus_find_device_by_id;
    bus->find_device_by_class = pci_bus_find_device_by_class;

}

