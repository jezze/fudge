#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "ide.h"

enum ide_register
{

    IDE_REGISTER_BUS0CONTROL0           = 0x03F6,
    IDE_REGISTER_BUS0CONTROL1           = 0x0376,
    IDE_REGISTER_BUS0DATA0              = 0x01F0,
    IDE_REGISTER_BUS0DATA1              = 0x0170,
    IDE_REGISTER_BUS1CONTROL0           = 0x03E6,
    IDE_REGISTER_BUS1CONTROL1           = 0x0366,
    IDE_REGISTER_BUS1DATA0              = 0x01E8,
    IDE_REGISTER_BUS1DATA1              = 0x0168

};

enum ide_control
{

    IDE_CONTROL_ATAPIEJECT              = 0x1B,
    IDE_CONTROL_PIO28READ               = 0x20,
    IDE_CONTROL_PIO48READ               = 0x24,
    IDE_CONTROL_DMA48READ               = 0x25,
    IDE_CONTROL_PIO28WRITE              = 0x30,
    IDE_CONTROL_PIO48WRITE              = 0x34,
    IDE_CONTROL_DMA48WRITE              = 0x35,
    IDE_CONTROL_ATAPI                   = 0xA0,
    IDE_CONTROL_IDATAPI                 = 0xA1,
    IDE_CONTROL_ATAPIREAD               = 0xA8,
    IDE_CONTROL_DMA28READ               = 0xC8,
    IDE_CONTROL_DMA28WRITE              = 0xCA,
    IDE_CONTROL_IDATA                   = 0xEC

};

enum ide_data
{

    IDE_DATA_FEATURE                    = 0x01,
    IDE_DATA_COUNT0                     = 0x02,
    IDE_DATA_LBA0                       = 0x03,
    IDE_DATA_LBA1                       = 0x04,
    IDE_DATA_LBA2                       = 0x05,
    IDE_DATA_SELECT                     = 0x06,
    IDE_DATA_COMMAND                    = 0x07,
    IDE_DATA_COUNT1                     = 0x08,
    IDE_DATA_LBA3                       = 0x09,
    IDE_DATA_LBA4                       = 0x0A,
    IDE_DATA_LBA5                       = 0x0B

};

enum ide_irq
{

    IDE_IRQ_PRIMARY                     = 0x0E,
    IDE_IRQ_SECONDARY                   = 0x0F

};

enum ide_status
{

    IDE_STATUS_ERROR                    = 0x01,
    IDE_STATUS_DRQ                      = 0x08,
    IDE_STATUS_SRV                      = 0x10,
    IDE_STATUS_DF                       = 0x20,
    IDE_STATUS_RDY                      = 0x40,
    IDE_STATUS_BUSY                     = 0x80

};

enum ide_id
{

    IDE_ID_TYPE                         = 0x00,
    IDE_ID_SERIAL                       = 0x0A,
    IDE_ID_MODEL                        = 0x1B,
    IDE_ID_CAP                          = 0x31,
    IDE_ID_VALID                        = 0x35,
    IDE_ID_LBA28MAX                     = 0x3C,
    IDE_ID_SUPPORT                      = 0x53,
    IDE_ID_LBA48MAX                     = 0x64

};

static struct base_driver driver;

static struct instance
{

    struct base_device device;
    struct ide_bus p;
    struct ide_bus s;

} instances[2];

static struct instance *find_instance(struct base_bus *bus, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct instance *instance = &instances[i];

        if (instance->device.bus == bus && instance->device.id == id)
            return instance;

    }

    return 0;

}

static void wait(struct ide_bus *bus)
{

    while (io_inb(bus->data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

static void ide_bus_sleep(struct ide_bus *bus)
{

    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);

}

static void ide_bus_select(struct ide_bus *bus, unsigned char operation, unsigned int slave)
{

    io_outb(bus->data + IDE_DATA_SELECT, operation | slave << 4);
    ide_bus_sleep(bus);

}

static void ide_bus_set_lba(struct ide_bus *bus, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(bus->data + IDE_DATA_COUNT0, count);
    io_outb(bus->data + IDE_DATA_LBA0, lba0);
    io_outb(bus->data + IDE_DATA_LBA1, lba1);
    io_outb(bus->data + IDE_DATA_LBA2, lba2);

}

static void ide_bus_set_lba2(struct ide_bus *bus, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(bus->data + IDE_DATA_COUNT1, count);
    io_outb(bus->data + IDE_DATA_LBA3, lba3);
    io_outb(bus->data + IDE_DATA_LBA4, lba4);
    io_outb(bus->data + IDE_DATA_LBA5, lba5);

}

static void ide_bus_set_command(struct ide_bus *bus, unsigned char command)
{

    io_outb(bus->data + IDE_DATA_COMMAND, command);

}

unsigned int ide_bus_get_status(struct ide_bus *bus)
{

    return io_inb(bus->data + IDE_DATA_COMMAND);

}

unsigned int ide_bus_read_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(bus->data);

    return 512;

}

static unsigned int ide_bus_read_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        ide_bus_sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            *out++ = io_inw(bus->data);

    }

    return i;

}

unsigned int ide_bus_write_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        io_outw(bus->data, *out++);

    return 512;

}

static unsigned int ide_bus_write_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        ide_bus_sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            io_outw(bus->data, *out++);

    }

    return i;

}

unsigned int ide_bus_read_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, count, sector, sector >> 8, sector >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_bus_read_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, count, sector, sector >> 8, sector >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);

}

unsigned int ide_bus_write_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, count, sector, sector >> 8, sector >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_bus_write_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, count, sector, sector >> 8, sector >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);

}

unsigned int ide_bus_read_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    ide_bus_set_lba(bus, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_bus_read_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    ide_bus_set_lba(bus, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);
    ide_bus_sleep(bus);

}

unsigned int ide_bus_write_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    ide_bus_set_lba(bus, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_bus_write_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    ide_bus_set_lba(bus, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);
    ide_bus_sleep(bus);

}

/*
static unsigned int detect(struct ide_bus *bus, unsigned int slave)
{

    unsigned char buffer[512];
    unsigned char status;
    unsigned short lba;

    ide_bus_select(bus, 0xA0, slave);
    ide_bus_set_lba(bus, 0, 0, 0, 0);
    ide_bus_set_command(bus, IDE_CONTROL_IDATA);
    ide_bus_read_blocks(bus, 1, buffer);

    status = io_inb(bus->data + IDE_DATA_COMMAND);

    if (!status)
        return 0;

    wait(bus);

    lba = (io_inb(bus->data + IDE_DATA_LBA2) << 8) | io_inb(bus->data + IDE_DATA_LBA1);

    if (lba == 0x0000)
        return IDE_DEVICE_TYPE_ATA;

    if (lba == 0xEB14)
        return IDE_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return IDE_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return IDE_DEVICE_TYPE_SATAPI;

    return 0;

}
*/

static void setup(struct base_bus *self)
{

}

static unsigned int device_next_p(struct base_bus *self, unsigned int id)
{

    if (id == 0)
        return IDE_DEVICE_TYPE_ATA;

    return 0;

}

static unsigned int device_next_s(struct base_bus *self, unsigned int id)
{

    return 0;

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    return (id) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

}

static unsigned int check(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_bus_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_bus_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_IDE;

}

static void attach(struct base_bus *bus, unsigned int id)
{

    struct instance *instance = find_instance(0, 0);
    unsigned int bar0 = pci_bus_ind(bus, id, PCI_CONFIG_BAR0);
    unsigned int bar1 = pci_bus_ind(bus, id, PCI_CONFIG_BAR1);
    unsigned int bar2 = pci_bus_ind(bus, id, PCI_CONFIG_BAR2);
    unsigned int bar3 = pci_bus_ind(bus, id, PCI_CONFIG_BAR3);
    unsigned int bar4 = pci_bus_ind(bus, id, PCI_CONFIG_BAR4);

    base_init_device(&instance->device, bus, id);
    base_init_bus(&instance->p.base, IDE_BUS_TYPE, "ide0", setup, device_next_p, device_irq);
    base_init_bus(&instance->s.base, IDE_BUS_TYPE, "ide1", setup, device_next_s, device_irq);

    instance->p.data = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
    instance->p.control = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
    instance->p.busmaster = (bar4 & 0xFFFFFFFC) + 0;
    instance->s.data = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
    instance->s.control = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
    instance->s.busmaster = (bar4 & 0xFFFFFFFC) + 8;

    base_register_bus(&instance->p.base);
    base_register_bus(&instance->s.base);

}

static void detach(struct base_bus *bus, unsigned int id)
{

}

void init()
{

    memory_clear(instances, sizeof (struct instance) * 2);
    base_init_driver(&driver, "ide", check, attach, detach);
    base_register_driver(&driver);

}

void destroy()
{

}

