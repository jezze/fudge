#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "ahci.h"

static struct base_driver driver;

static void driver_init()
{

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_SATA;

}

static void driver_attach(unsigned int id)
{

}

static void driver_detach(unsigned int id)
{

}

void module_init()
{

    base_initdriver(&driver, "ahci", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PCI_BUS_TYPE);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PCI_BUS_TYPE);

}

