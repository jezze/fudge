#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pci/pci.h>

#define REG_REVISION                    0x00
#define REG_CONTROL                     0x04
#define REG_STATUS                      0x08
#define REG_INTSTATUS                   0x0C
#define REG_INTENABLE                   0x10
#define REG_INTDISABLE                  0x14
#define REG_HCCA                        0x18
#define REG_PERIODCURRENT               0x1C
#define REG_CONTROLHEAD                 0x20
#define REG_CONTROLCURRENT              0x24
#define REG_BULKHEAD                    0x28
#define REG_BULKCURRENT                 0x2C
#define REG_DONEHEAD                    0x30
#define REG_FMINTERVAL                  0x34
#define REG_FMREMAINING                 0x38
#define REG_FMNUMBER                    0x3C
#define REG_PERIODICSTART               0x40
#define REG_THRESHOLD                   0x44
#define REG_RHDESCRIPTORA               0x48
#define REG_RHDESCRIPTORB               0x4C
#define REG_RHSTATUS                    0x50

static struct base_driver driver;

static void driver_init(unsigned int id)
{

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x10;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

}

static void driver_detach(unsigned int id)
{

}

void module_init(void)
{

    base_initdriver(&driver, "ohci", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

