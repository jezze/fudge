#define IDE_BUS_TYPE                    0x0004
#define IDE_DEVICE_TYPE_UNKNOWN         0x00
#define IDE_DEVICE_TYPE_ATA             0x01
#define IDE_DEVICE_TYPE_ATAPI           0x02
#define IDE_DEVICE_TYPE_SATA            0x03
#define IDE_DEVICE_TYPE_SATAPI          0x04

unsigned char ide_getstatus(unsigned int id);
unsigned short ide_getirq(unsigned int id);
unsigned int ide_rblock(unsigned int id, unsigned int count, void *buffer);
unsigned int ide_wblock(unsigned int id, unsigned int count, void *buffer);
void ide_rlba28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count);
void ide_wlba28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count);
void ide_rlba48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count);
void ide_wlba48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count);
