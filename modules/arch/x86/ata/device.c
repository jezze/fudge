#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static void configure_ata(struct ata_device *self)
{

    unsigned short buffer[256];

    self->bus->read_blocks(self->bus, 1, buffer);

    unsigned int lba48 = buffer[ATA_ID_SUPPORT] & (1 << 10);

    self->lba28Max = (buffer[ATA_ID_LBA28MAX] << 16) | buffer[ATA_ID_LBA28MAX + 1];

    if (lba48)
    {

        self->lba48MaxLow = (buffer[ATA_ID_LBA48MAX + 0] << 16) | buffer[ATA_ID_LBA48MAX + 1];
        self->lba48MaxHigh = (buffer[ATA_ID_LBA48MAX + 2] << 16) | buffer[ATA_ID_LBA48MAX + 3];

    }

    unsigned int i;

    char *model = (char *)&buffer[ATA_ID_MODEL];

    for (i = 0; i < 40; i++)
        self->model[i] = model[i + 1 - ((i & 1) << 1)];

    self->model[40] = '\0';

    for (i = 39; i > 0; i--)
    {

        if (self->model[i] == ' ')
            self->model[i] = '\0';
        else
            break;

    }

}

static void configure_atapi(struct ata_device *self)
{

    unsigned short buffer[256];

    self->bus->set_command(self->bus, ATA_COMMAND_ID_ATAPI);
    self->bus->read_blocks(self->bus, 1, buffer);

}

static unsigned int read_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO28_READ);

    return self->bus->read_blocks(self->bus, count, buffer) * 512;

}

static void read_lba28_async(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO28_READ);
    self->bus->sleep(self->bus);

}

static unsigned int write_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO28_WRITE);

    return self->bus->write_blocks(self->bus, count, buffer) * 512;

}

static void write_lba28_async(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO28_WRITE);
    self->bus->sleep(self->bus);

}

static unsigned int read_lba48(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO48_READ);

    return self->bus->read_blocks(self->bus, count, buffer) * 512;

}

static void read_lba48_async(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO48_READ);
    self->bus->sleep(self->bus);

}

static unsigned int write_lba48(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO48_WRITE);

    return self->bus->write_blocks(self->bus, count, buffer) * 512;

}

static void write_lba48_async(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, ATA_COMMAND_PIO48_WRITE);
    self->bus->sleep(self->bus);

}

void ata_device_init(struct ata_device *device, struct ata_bus *bus, unsigned int irq, unsigned int slave, unsigned int type)
{

    memory_clear(device, sizeof (struct ata_device));

    modules_device_init(&device->base, ATA_DEVICE_TYPE, "ata:0:0");

    device->bus = bus;
    device->type = type;
    device->irq = irq;
    device->slave = slave;
    device->configure_ata = configure_ata;
    device->configure_atapi = configure_atapi;
    device->lba28Max = 0;
    device->read_lba28 = read_lba28;
    device->read_lba28_async = read_lba28_async;
    device->write_lba28 = write_lba28;
    device->write_lba28_async = write_lba28_async;
    device->lba48MaxLow = 0;
    device->lba48MaxHigh = 0;
    device->read_lba48 = read_lba48;
    device->read_lba48_async = read_lba48_async;
    device->write_lba48 = write_lba48;
    device->write_lba48_async = write_lba48_async;

}

