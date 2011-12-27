#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
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

void rtl8139_driver_start(struct rtl8139_driver *self)
{

    poweron(self);
    reset(self);
    get_mac(self);
    set_rx(self, 0x0F);
    set_interrupt_flags(self, 0x05);
    enable(self);

}

void rtl8139_driver_init(struct rtl8139_driver *driver)
{

    modules_driver_init(&driver->base, RTL8139_DRIVER_TYPE);
    driver->start = rtl8139_driver_start;

}

