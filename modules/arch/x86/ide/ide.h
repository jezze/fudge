#define IDE_BUS_TYPE                    0x0004
#define IDE_DEVICE_TYPE_UNKNOWN         0x00
#define IDE_DEVICE_TYPE_ATA             0x01
#define IDE_DEVICE_TYPE_ATAPI           0x02
#define IDE_DEVICE_TYPE_SATA            0x03
#define IDE_DEVICE_TYPE_SATAPI          0x04

struct ide_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    unsigned char model[41];
    unsigned int lba28Max;
    unsigned int lba48MaxLow;
    unsigned int lba48MaxHigh;

};

unsigned int ide_bus_read_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
void ide_bus_read_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_bus_write_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
void ide_bus_write_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_bus_read_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_bus_read_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
unsigned int ide_bus_write_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_bus_write_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_configure_ata(struct ide_bus *bus);
void ide_configure_atapi(struct ide_bus *bus);
