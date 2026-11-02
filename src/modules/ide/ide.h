#define IDE_BUS                         0x0004
#define IDE_PM                          0x01
#define IDE_PS                          0x02
#define IDE_SM                          0x03
#define IDE_SS                          0x04
#define IDE_PACKETSIZE                  12
#define IDE_DMA_BUSY                    0
#define IDE_DMA_DONE                    1
#define IDE_DMA_FAILED                  2

unsigned char ide_getstatus(unsigned int id);
unsigned int ide_identify(unsigned int id, void *buffer);
unsigned int ide_identifypacket(unsigned int id, void *buffer);
unsigned short ide_getirq(unsigned int id);
unsigned int ide_wait(unsigned int id);
unsigned int ide_rblock(unsigned int id, void *buffer);
unsigned int ide_wblock(unsigned int id, void *buffer);
void ide_rpio28(unsigned int id, unsigned int count, unsigned int sector);
void ide_wpio28(unsigned int id, unsigned int count, unsigned int sector);
void ide_rpio48(unsigned int id, unsigned int count, unsigned int sector);
void ide_wpio48(unsigned int id, unsigned int count, unsigned int sector);
unsigned int ide_packet(unsigned int id, void *packet, unsigned int limit, unsigned int dma);
unsigned char ide_getreason(unsigned int id);
unsigned int ide_getbytecount(unsigned int id);
void ide_rdata(unsigned int id, void *buffer, unsigned int count);
unsigned int ide_lock(unsigned int id);
void ide_unlock(unsigned int id);
unsigned int ide_hasdma(unsigned int id);
unsigned int ide_setdma(unsigned int id, unsigned long address, unsigned int count, unsigned int read);
void ide_startdma(unsigned int id);
unsigned int ide_checkdma(unsigned int id);
void ide_rdma48(unsigned int id, unsigned int count, unsigned int sector);
void ide_wdma48(unsigned int id, unsigned int count, unsigned int sector);
