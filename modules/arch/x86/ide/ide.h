#define IDE_BUS_TYPE                    0x9001
#define IDE_DEVICE_TYPE_UNKNOWN         0x00
#define IDE_DEVICE_TYPE_ATA             0x01
#define IDE_DEVICE_TYPE_ATAPI           0x02
#define IDE_DEVICE_TYPE_SATA            0x03
#define IDE_DEVICE_TYPE_SATAPI          0x04

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

struct ide_partition
{

    unsigned char boot;
    unsigned char headbase;
    unsigned char sectorbase;
    unsigned char cylinderbase;
    unsigned char systemid;
    unsigned char headlimit;
    unsigned char sectorlimit;
    unsigned char cylinderlimit;
    unsigned char sectorlba[4];
    unsigned char sectortotal[4];

};

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
