#include <kernel/arch/x86/io.h>
#include <kernel/irq.h>
#include <kernel/log.h>
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

static void set_rx(struct rtl8139_driver *driver)
{

    io_outd(driver->io + RTL8139_REGISTER_RBSTART, (unsigned int)driver->rx);
    io_outd(driver->io + RTL8139_REGISTER_CBR, 0);
    io_outd(driver->io + RTL8139_REGISTER_CAPR, 0);

}

static void set_tx(struct rtl8139_driver *driver)
{

}

static void set_interrupt_flags(struct rtl8139_driver *driver, unsigned short flags)
{

    io_outw(driver->io + RTL8139_REGISTER_IMR, flags);

}

static void get_mac(struct rtl8139_driver *driver)
{

    driver->mac[0] = io_inb(driver->io + RTL8139_REGISTER_IDR0);
    driver->mac[1] = io_inb(driver->io + RTL8139_REGISTER_IDR1);
    driver->mac[2] = io_inb(driver->io + RTL8139_REGISTER_IDR2);
    driver->mac[3] = io_inb(driver->io + RTL8139_REGISTER_IDR3);
    driver->mac[4] = io_inb(driver->io + RTL8139_REGISTER_IDR4);
    driver->mac[5] = io_inb(driver->io + RTL8139_REGISTER_IDR5);

    log_write("[rtl8139] Mac: %x:%x:%x:%x:%x:%x\n", driver->mac[0], driver->mac[1], driver->mac[2], driver->mac[3], driver->mac[4], driver->mac[5]);

}

static void read(struct rtl8139_driver *driver)
{

//    unsigned int cbr = inw(driver->io + RTL8139_REGISTER_CBR);
//    unsigned int capr = inw(driver->io + RTL8139_REGISTER_CAPR);

    log_write("[rtl8139] IRQ Read\n");

}

static void write(struct rtl8139_driver *driver)
{

    log_write("[rtl8139] IRQ Write\n");

}

static void handle_irq(struct modules_device *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;

    unsigned int status = io_inw(driver->io + RTL8139_REGISTER_ISR);

    if (status & RTL8139_ISR_FLAG_ROK)
    {

        read(driver);

        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_FLAG_ROK);

    }

    if (status & RTL8139_ISR_FLAG_TOK)
    {

        write(driver);

        io_outw(driver->io + RTL8139_REGISTER_ISR, RTL8139_ISR_FLAG_TOK);

    }

}

static void rtl8139_driver_start(struct modules_driver *self)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self;

    poweron(driver);
    reset(driver);
    set_interrupt_flags(driver, RTL8139_ISR_FLAG_ROK | RTL8139_ISR_FLAG_TOK);
    set_rx(driver);
    set_tx(driver);

    io_outd(driver->io + RTL8139_REGISTER_RCR, 0x0F);
    io_outw(driver->io + RTL8139_REGISTER_CR, 0x0C);

    get_mac(driver);

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

