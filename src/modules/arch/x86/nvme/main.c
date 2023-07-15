#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pci/pci.h>
#include "nvme.h"

static struct base_driver driver;
static struct base_bus bus;
static unsigned int mmio;

static void bus_setup(void)
{

    DEBUG_FMT0(DEBUG_INFO, "NVME INIT\n");

}

static unsigned int bus_next(unsigned int id)
{

    return 0;

}

static void driver_init(unsigned int id)
{

    base_initbus(&bus, NVME_BUS, "nvme", bus_setup, bus_next);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_NVME;

}

static void driver_reset(unsigned int id)
{

    mmio = pci_ind(id, PCI_CONFIG_BAR5);

}

static void driver_attach(unsigned int id)
{

    base_registerbus(&bus);

}

static void driver_detach(unsigned int id)
{

    base_unregisterbus(&bus);

}

void module_init(void)
{

    base_initdriver(&driver, "nvme", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

