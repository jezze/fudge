#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static void poweron(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CONFIG1, 0x00);

}

static void reset(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CR, 0x10);

    while (io_inb(self->io + RTL8139_REGISTER_CR) & 0x10);

}

static void enable(struct rtl8139_driver *self)
{

    io_outb(self->io + RTL8139_REGISTER_CR, 0x0C);

}

static void setup_interrupts(struct rtl8139_driver *self, unsigned short flags)
{

    io_outw(self->io + RTL8139_REGISTER_IMR, flags);

}

static void setup_receiver(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_REGISTER_RBSTART, (unsigned int)self->rx);
    io_outd(self->io + RTL8139_REGISTER_RCR, 0x0000000F);

}

static void setup_transmitter(struct rtl8139_driver *self)
{

    io_outd(self->io + RTL8139_REGISTER_TSAD0, (unsigned int)self->tx0);
    io_outd(self->io + RTL8139_REGISTER_TSAD1, (unsigned int)self->tx1);
    io_outd(self->io + RTL8139_REGISTER_TSAD2, (unsigned int)self->tx2);
    io_outd(self->io + RTL8139_REGISTER_TSAD3, (unsigned int)self->tx3);

}

static unsigned int read(struct rtl8139_driver *self, void *buffer)
{

    unsigned short current = io_inw(self->io + RTL8139_REGISTER_CAPR) + 0x10;
    unsigned short end = io_inw(self->io + RTL8139_REGISTER_CBR);

    struct rtl8139_header *header = (struct rtl8139_header *)(self->rx + current);

    memory_copy(buffer, (void *)self->rx + current + 4, header->length);

    current += (header->length + 4 + 3) & ~3;

    io_outw(self->io + RTL8139_REGISTER_CAPR, current - 0x10);

    return header->length;

}

static unsigned int write(struct rtl8139_driver *self, unsigned int count, void *buffer)
{

    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (self->txp)
    {

        case 0:

            memory_copy(self->tx0, buffer, count);
            io_outd(self->io + RTL8139_REGISTER_TSD0, status);

            break;

        case 1:

            memory_copy(self->tx1, buffer, count);
            io_outd(self->io + RTL8139_REGISTER_TSD1, status);

            break;

        case 2:

            memory_copy(self->tx2, buffer, count);
            io_outd(self->io + RTL8139_REGISTER_TSD2, status);

            break;

        case 3:

            memory_copy(self->tx3, buffer, count);
            io_outd(self->io + RTL8139_REGISTER_TSD3, status);

            break;

    }

    self->txp++;
    self->txp %= 4;

    return count;

}

static void handle_irq(struct modules_device *device)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)device->driver;

    unsigned int status = io_inw(driver->io + RTL8139_REGISTER_ISR);

    if (status & RTL8139_ISR_FLAG_ROK)
    {

        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_FLAG_ROK);
        event_raise(EVENT_IRQ_NETWORK);

    }

    if (status & RTL8139_ISR_FLAG_TOK)
    {

        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_FLAG_TOK);

    }

}

static void start(struct modules_driver *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;

    poweron(driver);
    reset(driver);
    setup_interrupts(driver, RTL8139_ISR_FLAG_ROK | RTL8139_ISR_FLAG_TOK);
    setup_receiver(driver);
    setup_transmitter(driver);
    enable(driver);

    driver->mac[0] = io_inb(driver->io + RTL8139_REGISTER_IDR0);
    driver->mac[1] = io_inb(driver->io + RTL8139_REGISTER_IDR1);
    driver->mac[2] = io_inb(driver->io + RTL8139_REGISTER_IDR2);
    driver->mac[3] = io_inb(driver->io + RTL8139_REGISTER_IDR3);
    driver->mac[4] = io_inb(driver->io + RTL8139_REGISTER_IDR4);
    driver->mac[5] = io_inb(driver->io + RTL8139_REGISTER_IDR5);

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;
    struct pci_device *pciDevice = (struct pci_device *)device;

    driver->io = (pciDevice->configuration.bar0 & ~1);

    irq_register_routine(pciDevice->configuration.interruptline, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *pciDevice = (struct pci_device *)device;

    return pciDevice->configuration.vendorid == 0x10EC && pciDevice->configuration.deviceid == 0x8139;

}

void rtl8139_driver_init(struct rtl8139_driver *driver)
{

    modules_driver_init(&driver->base, RTL8139_DRIVER_TYPE);

    driver->base.start = start;
    driver->base.attach = attach;
    driver->base.check = check;
    driver->read = read;
    driver->write = write;
    driver->txp = 0;

}

