#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/ata/ata.h>

static void configure_ata_device(struct ata_device *device, unsigned short *buffer)
{

    unsigned int lba48 = buffer[ATA_ID_SUPPORT] & (1 << 10);

    device->lba28Max = (buffer[ATA_ID_LBA28MAX] << 16) | buffer[ATA_ID_LBA28MAX + 1];

    if (lba48)
    {

        device->lba48MaxLow = (buffer[ATA_ID_LBA48MAX + 0] << 16) | buffer[ATA_ID_LBA48MAX + 1];
        device->lba48MaxHigh = (buffer[ATA_ID_LBA48MAX + 2] << 16) | buffer[ATA_ID_LBA48MAX + 3];

    }

    unsigned int i;

    char *model = (char *)&buffer[ATA_ID_MODEL];

    for (i = 0; i < 40; i++)
        device->model[i] = model[i + 1 - ((i & 1) << 1)];

    device->model[40] = '\0';

    for (i = 39; i > 0; i--)
    {

        if (device->model[i] == ' ')
            device->model[i] = '\0';
        else
            break;

    }

}

static void configure_atapi_device(struct ata_device *device, unsigned short *buffer)
{

}

struct pci_device *find_pci_device()
{

    unsigned int i;
    struct pci_bus *bus;

    for (i = 0; (bus = (struct pci_bus *)modules_get_bus(PCI_BUS_TYPE, i)); i++)
    {

        struct pci_device *device = bus->find_device_by_class(bus, 0x01, 0x01, 0);

        if (device)
            return device;

    }

    return 0;

}

static unsigned int ata_bus_read_blocks(struct ata_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        bus->sleep(bus);
        bus->wait(bus);

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(bus->data);

    }

    return i;

}

static void ata_bus_sleep(struct ata_bus *self)
{

    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);

}

static void ata_bus_wait(struct ata_bus *self)
{

    while (io_inb(self->data + ATA_DATA_COMMAND) & ATA_STATUS_FLAG_BUSY);

}

static void ata_bus_select(struct ata_bus *self, unsigned char operation, unsigned int secondary)
{

    io_outb(self->data + ATA_DATA_SELECT, operation | secondary);
    self->sleep(self);

}

static void ata_bus_set_lba(struct ata_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(self->data + ATA_DATA_COUNT0, count);
    io_outb(self->data + ATA_DATA_LBA0, lba0);
    io_outb(self->data + ATA_DATA_LBA1, lba1);
    io_outb(self->data + ATA_DATA_LBA2, lba2);

}

static void ata_bus_set_lba2(struct ata_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(self->data + ATA_DATA_COUNT1, count);
    io_outb(self->data + ATA_DATA_LBA3, lba3);
    io_outb(self->data + ATA_DATA_LBA4, lba4);
    io_outb(self->data + ATA_DATA_LBA5, lba5);

}

static void ata_bus_set_command(struct ata_bus *self, unsigned char command)
{

    io_outb(self->data + ATA_DATA_COMMAND, command);

}

static unsigned int ata_bus_detect(struct ata_bus *self, unsigned int secondary, void *buffer)
{

    self->select(self, 0xA0, secondary);
    self->set_lba(self, 0, 0, 0, 0);
    self->set_command(self, ATA_COMMAND_ID_ATA);

    unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

    if (!status)
        return 0;

    self->wait(self);

    unsigned short lba = (io_inb(self->data + ATA_DATA_LBA2) << 8) | io_inb(self->data + ATA_DATA_LBA1);

    if (lba == 0x0000)
    {

        self->read_blocks(self, 1, buffer);

        return ATA_DEVICE_TYPE_ATA;

    }

    if (lba == 0xEB14)
    {

        self->set_command(self, ATA_COMMAND_ID_ATAPI);
        self->read_blocks(self, 1, buffer);

        return ATA_DEVICE_TYPE_ATAPI;

    }

    if (lba == 0xC33C)
        return ATA_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return ATA_DEVICE_TYPE_SATAPI;

    return 0;

}

static struct ata_device *ata_bus_find_device(struct ata_bus *self, unsigned int type, unsigned int index)
{

    if (index == 0 && self->primary.type == type)
        return &self->primary;

    if (index < 2 && self->secondary.type == type)
        return &self->secondary;

    return 0;

}

unsigned int ata_bus_check_module(struct modules_module *module)
{

    if (!modules_is_bus(module))
        return 0;

    struct modules_bus *bus = (struct modules_bus *)module;

    return bus->type == ATA_BUS_TYPE;

}

void ata_bus_init(struct ata_bus *bus, unsigned int control, unsigned int data)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);
    bus->control = control;
    bus->data = data;
    bus->sleep = ata_bus_sleep;
    bus->wait = ata_bus_wait;
    bus->select = ata_bus_select;
    bus->set_lba = ata_bus_set_lba;
    bus->set_lba2 = ata_bus_set_lba2;
    bus->set_command = ata_bus_set_command;
    bus->detect = ata_bus_detect;
    bus->find_device = ata_bus_find_device;
    bus->read_blocks = ata_bus_read_blocks;

    unsigned short buffer[256];

    unsigned int type;

    if ((type = bus->detect(bus, 0 << 4, buffer)))
    {

        ata_device_init(&bus->primary, bus, 0 << 4, type);

        if (bus->primary.type == ATA_DEVICE_TYPE_ATA)
        {

            bus->primary.pciDevice = find_pci_device();
            configure_ata_device(&bus->primary, buffer);

        }

        if (bus->primary.type == ATA_DEVICE_TYPE_ATAPI)
            configure_atapi_device(&bus->primary, buffer);

        modules_register_device(&bus->primary.base);

    }

    if ((type = bus->detect(bus, 1 << 4, buffer)))
    {

        ata_device_init(&bus->secondary, bus, 1 << 4, type);

        if (bus->secondary.type == ATA_DEVICE_TYPE_ATA)
        {

            bus->secondary.pciDevice = find_pci_device();
            configure_ata_device(&bus->secondary, buffer);

        }

        if (bus->secondary.type == ATA_DEVICE_TYPE_ATAPI)
            configure_atapi_device(&bus->secondary, buffer);

        modules_register_device(&bus->secondary.base);

    }

}

