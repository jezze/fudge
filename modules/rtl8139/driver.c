#include <kernel/arch/x86/io.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static void poweron(struct rtl8139_driver *driver)
{

    io_outb(driver->io + RTL8139_REGISTER_CONFIG1, 0x00);

}

static void reset(struct rtl8139_driver *driver)
{

    io_outb(driver->io + RTL8139_REGISTER_CR, 0x10);

    while (io_inb(driver->io + RTL8139_REGISTER_CR) & 0x10);

}

static void set_rx(struct rtl8139_driver *driver, unsigned int rcr)
{

    io_outd(driver->io + RTL8139_REGISTER_RBSTART, (unsigned int)driver->rx);
    io_outd(driver->io + RTL8139_REGISTER_RCR, rcr);

}

static void set_interrupt_flags(struct rtl8139_driver *driver, unsigned short flags)
{

    io_outw(driver->io + RTL8139_REGISTER_IMR, flags);

}

static void enable(struct rtl8139_driver *driver)
{

    io_outw(driver->io + RTL8139_REGISTER_CR, 0x0C);

}

static void get_mac(struct rtl8139_driver *driver)
{

    driver->mac[0] = io_inb(driver->io + RTL8139_REGISTER_IDR0);
    driver->mac[1] = io_inb(driver->io + RTL8139_REGISTER_IDR1);
    driver->mac[2] = io_inb(driver->io + RTL8139_REGISTER_IDR2);
    driver->mac[3] = io_inb(driver->io + RTL8139_REGISTER_IDR3);
    driver->mac[4] = io_inb(driver->io + RTL8139_REGISTER_IDR4);
    driver->mac[5] = io_inb(driver->io + RTL8139_REGISTER_IDR5);

}

static void handle_irq(struct modules_device *self)
{

}

static void rtl8139_driver_start(struct modules_driver *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;

    poweron(driver);
    reset(driver);
    get_mac(driver);
    set_rx(driver, 0x0F);
    set_interrupt_flags(driver, 0x05);
    enable(driver);

}

static void rtl8139_driver_attach(struct modules_driver *self, struct modules_device *device)
{

    device->driver = self;

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;
    struct pci_device *pciDevice = (struct pci_device *)device;

    driver->io = (pciDevice->configuration.bar0 & ~1);

    irq_register_routine(pciDevice->configuration.interruptline, device, handle_irq);

}

static unsigned int rtl8139_driver_check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *pciDevice = (struct pci_device *)device;

    return pciDevice->configuration.vendorid == 0x10EC && pciDevice->configuration.deviceid == 0x8139;

}

void rtl8139_driver_init(struct rtl8139_driver *driver)
{

    modules_driver_init(&driver->base, RTL8139_DRIVER_TYPE);
    driver->base.start = rtl8139_driver_start;
    driver->base.attach = rtl8139_driver_attach;
    driver->base.check = rtl8139_driver_check;

}

