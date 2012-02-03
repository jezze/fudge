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

static void enable(struct rtl8139_driver *driver)
{

    io_outw(driver->io + RTL8139_REGISTER_CR, 0x0C);

}

static void setup_interrupts(struct rtl8139_driver *driver, unsigned short flags)
{

    io_outw(driver->io + RTL8139_REGISTER_IMR, flags);

}

static void setup_receiver(struct rtl8139_driver *driver)
{

    io_outd(driver->io + RTL8139_REGISTER_RBSTART, (unsigned int)driver->rx);
    io_outd(driver->io + RTL8139_REGISTER_CBR, 0);
    io_outd(driver->io + RTL8139_REGISTER_CAPR, 0);
    io_outd(driver->io + RTL8139_REGISTER_RCR, 0x0F);

}

static void setup_transmitter(struct rtl8139_driver *driver)
{

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

static void read_arp(unsigned char *data)
{

    log_write("- ARP Operation: %d\n", data[6] << 8 | data[7]);

}

static void read_icmpv6(unsigned char *data)
{

    log_write("- ICMPv6 Type: %d\n", data[0]);
    log_write("- ICMPv6 Code: %d\n", data[1]);

}

static void read_ipv6(unsigned char *data)
{

    log_write("- IPv6 Version: %d\n", data[0] >> 4);
    log_write("- IPv6 Length:  %d\n", data[4]);
    log_write("- IPv6 Next:  %d\n", data[6]);
    log_write("- IPv6 Source:  %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n", data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23]);
    log_write("- IPv6 Dest:    %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n", data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31], data[32], data[33], data[34], data[35], data[36], data[37], data[38], data[39]);

    if (data[6] == 58)
        read_icmpv6(data + 40);

}

static void read_frame(unsigned char *data)
{

    log_write("- Frame Dest:   %x:%x:%x:%x:%x:%x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    log_write("- Frame Source: %x:%x:%x:%x:%x:%x\n", data[6], data[7], data[8], data[9], data[10], data[11]);
    log_write("- Frame Type:   0x%x%x\n", data[12], data[13]);

    unsigned short type = *(unsigned short *)(data + 12);

    if (data[12] == 0x08 && data[13] == 0x06)
        read_arp(data + 14);

    if (data[12] == 0x86 && data[13] == 0xDD)
        read_ipv6(data + 14);

}

static void read(struct rtl8139_driver *driver)
{

    unsigned short current = io_inw(driver->io + RTL8139_REGISTER_CAPR) + 0x10;
    unsigned short end = io_inw(driver->io + RTL8139_REGISTER_CBR);

    while (current < end)
    {

        struct rtl8139_header *header = (struct rtl8139_header *)(driver->rx + current);

        log_write("[rtl8139]   0x%x Flags:0x%x Length:0x%x\n", current, header->flags, header->length);

        read_frame((unsigned char *)(driver->rx + current + 4));

        current += (header->length + 4 + 3) & ~3;

    }

    io_outw(driver->io + RTL8139_REGISTER_CAPR, current - 0x10);

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
    setup_interrupts(driver, RTL8139_ISR_FLAG_ROK | RTL8139_ISR_FLAG_TOK);
    setup_receiver(driver);
    setup_transmitter(driver);
    enable(driver);
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

