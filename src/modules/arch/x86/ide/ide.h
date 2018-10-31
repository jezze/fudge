#define IDE_BUS                         0x0004
#define IDE_UNKNOWN                     0x00
#define IDE_ATA                         0x01
#define IDE_ATAPI                       0x02
#define IDE_SATA                        0x03
#define IDE_SATAPI                      0x04

unsigned char ide_getstatus(unsigned int id);
unsigned short ide_getirq(unsigned int id);
unsigned int ide_rblock(unsigned int id, void *buffer);
unsigned int ide_wblock(unsigned int id, void *buffer);
void ide_rpio28(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector);
void ide_wpio28(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector);
void ide_rpio48(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector);
void ide_wpio48(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector);
