#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

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

void rtl8139_driver_init(struct rtl8139_driver *driver, struct pci_device *device)
{

    driver->base.device = &device->base;
    driver->io = (device->configuration.bar0 & ~1);

    poweron(driver);
    reset(driver);
    get_mac(driver);
    set_rx(driver, 0x0F);
    set_interrupt_flags(driver, 0x05);
    enable(driver);

}

static void handle_irq()
{

}

void init()
{

    struct pci_bus *bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE);

    if (!bus)
        return;

    struct pci_device *device = bus->find_device(bus, 0x8139);

    if (!device)
        return;

    rtl8139_driver_init(&driver, device);

    kernel_register_irq(device->configuration.interruptline, handle_irq);

}

void destroy()
{

    struct pci_device *device = (struct pci_device *)driver.base.device;

    kernel_unregister_irq(device->configuration.interruptline);

}

