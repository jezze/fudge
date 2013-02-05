#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <net/net.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "rtl8139.h"

static void poweron(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_CONFIG1, 0x00);

}

static void reset(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_CR, 0x10);

    while (io_inb(self->io + RTL8139_CR) & 0x10);

}

static void enable(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_CR, 0x0C);

}

static void setup_interrupts(struct rtl8139_driver *self, unsigned short flags)
{

    io_outw(self->io + RTL8139_IMR, flags);

}

static void setup_receiver(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_RBSTART, (unsigned int)self->rx);
    io_outd(self->io + RTL8139_RCR, 0x0000000F);

}

static void setup_transmitter(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_TSAD0, (unsigned int)self->tx0);
    io_outd(self->io + RTL8139_TSAD1, (unsigned int)self->tx1);
    io_outd(self->io + RTL8139_TSAD2, (unsigned int)self->tx2);
    io_outd(self->io + RTL8139_TSAD3, (unsigned int)self->tx3);

}

static void handle_irq(struct base_device *device)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)device->driver;
    unsigned int status = io_inw(driver->io + RTL8139_ISR);

    if (status & RTL8139_ISR_ROK)
    {

        unsigned short current = io_inw(driver->io + RTL8139_CAPR) + 0x10;
        struct rtl8139_header *header = (struct rtl8139_header *)(driver->rx + current);

/*
        net_handle_read(&driver->interface, header->length, driver->rx + current + 4);
*/

        current += (header->length + 4 + 3) & ~3;

        io_outw(driver->io + RTL8139_CAPR, current - 0x10);
        io_outw(driver->io + RTL8139_ISR, RTL8139_ISR_ROK);

    }

    if (status & RTL8139_ISR_TOK)
        io_outw(driver->io + RTL8139_ISR, RTL8139_ISR_TOK);

}

static void start(struct base_driver *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;

    poweron(driver);
    reset(driver);
    setup_interrupts(driver, RTL8139_ISR_ROK | RTL8139_ISR_TOK);
    setup_receiver(driver);
    setup_transmitter(driver);
    enable(driver);

    driver->interface.mac[0] = io_inb(driver->io + RTL8139_IDR0);
    driver->interface.mac[1] = io_inb(driver->io + RTL8139_IDR1);
    driver->interface.mac[2] = io_inb(driver->io + RTL8139_IDR2);
    driver->interface.mac[3] = io_inb(driver->io + RTL8139_IDR3);
    driver->interface.mac[4] = io_inb(driver->io + RTL8139_IDR4);
    driver->interface.mac[5] = io_inb(driver->io + RTL8139_IDR5);

}

static void attach(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;
    struct rtl8139_driver *driver = (struct rtl8139_driver *)device->driver;
    unsigned int bar0 = pci_bus_ind(pciDevice->bus, pciDevice->address, PCI_CONFIG_BAR0);
    unsigned int irq = pci_bus_inb(pciDevice->bus, pciDevice->address, PCI_CONFIG_IRQ_LINE);

    driver->io = bar0 & ~1;

    pic_set_routine(irq, device, handle_irq);

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_bus_inw(pciDevice->bus, pciDevice->address, PCI_CONFIG_VENDOR) == RTL8139_PCI_VENDOR && pci_bus_inw(pciDevice->bus, pciDevice->address, PCI_CONFIG_DEVICE) == RTL8139_PCI_DEVICE;

}

static unsigned int send(struct net_interface *self, unsigned int count, void *buffer)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;
    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (driver->txp)
    {

        case 0:

            memory_write(driver->tx0, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_TSD0, status);

            break;

        case 1:

            memory_write(driver->tx1, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_TSD1, status);

            break;

        case 2:

            memory_write(driver->tx2, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_TSD2, status);

            break;

        case 3:

            memory_write(driver->tx3, 0x800, buffer, count, 0);
            io_outd(driver->io + RTL8139_TSD3, status);

            break;

    }

    driver->txp++;
    driver->txp %= 4;

    return count;

}

void rtl8139_init_driver(struct rtl8139_driver *driver)
{

    memory_clear(driver, sizeof (struct rtl8139_driver));
    base_init_driver(&driver->base, "rtl8139", start, check, attach);
    net_init_interface(&driver->interface, &driver->base, send);

}

