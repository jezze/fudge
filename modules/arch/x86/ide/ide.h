#define IDE_BUS_TYPE                    0x0004
#define IDE_DEVICE_TYPE_UNKNOWN         0x00
#define IDE_DEVICE_TYPE_ATA             0x01
#define IDE_DEVICE_TYPE_ATAPI           0x02
#define IDE_DEVICE_TYPE_SATA            0x03
#define IDE_DEVICE_TYPE_SATAPI          0x04

unsigned int ide_get_status(struct base_bus *bus);
unsigned int ide_read_lba28(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
void ide_read_lba28_async(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_write_lba28(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
void ide_write_lba28_async(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_read_lba48(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_read_lba48_async(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
unsigned int ide_write_lba48(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_write_lba48_async(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
unsigned int ide_read_block(struct base_bus *bus, unsigned int count, void *buffer);
unsigned int ide_write_block(struct base_bus *bus, unsigned int count, void *buffer);
