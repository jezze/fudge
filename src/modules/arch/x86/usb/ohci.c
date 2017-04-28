#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pci/pci.h>

#define REGISTERREVISION                0x00
#define REGISTERCONTROL                 0x04
#define REGISTERSTATUS                  0x08
#define REGISTERINTSTATUS               0x0C
#define REGISTERINTENABLE               0x10
#define REGISTERINTDISABLE              0x14
#define REGISTERHCCA                    0x18
#define REGISTERPERIODCURRENT           0x1C
#define REGISTERCONTROLHEAD             0x20
#define REGISTERCONTROLCURRENT          0x24
#define REGISTERBULKHEAD                0x28
#define REGISTERBULKCURRENT             0x2C
#define REGISTERDONEHEAD                0x30
#define REGISTERFMINTERVAL              0x34
#define REGISTERFMREMAINING             0x38
#define REGISTERFMNUMBER                0x3C
#define REGISTERPERIODICSTART           0x40
#define REGISTERTHRESHOLD               0x44
#define REGISTERRHDESCRIPTORA           0x48
#define REGISTERRHDESCRIPTORB           0x4C
#define REGISTERRHSTATUS                0x50

static struct base_driver driver;

static void driver_init(void)
{

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x10;

}

static void driver_attach(unsigned int id)
{

    /*
    unsigned int bar0 = pci_ind(id, PCI_CONFIG_BAR0);
    */

}

static void driver_detach(unsigned int id)
{

}

void module_init(void)
{

    base_initdriver(&driver, "ohci", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

