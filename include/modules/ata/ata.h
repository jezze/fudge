#ifndef MODULES_ATA_H
#define MODULES_ATA_H

#define ATA_COMMAND_ATAPI_EJECT 0x1B
#define ATA_COMMAND_PIO28_READ  0x20
#define ATA_COMMAND_PIO48_READ  0x24
#define ATA_COMMAND_DMA48_READ  0x25
#define ATA_COMMAND_PIO28_WRITE 0x30
#define ATA_COMMAND_PIO48_WRITE 0x34
#define ATA_COMMAND_DMA48_WRITE 0x35
#define ATA_COMMAND_ATAPI       0xA0
#define ATA_COMMAND_ID_ATAPI    0xA1
#define ATA_COMMAND_ATAPI_READ  0xA8
#define ATA_COMMAND_DMA28_READ  0xC8
#define ATA_COMMAND_DMA28_WRITE 0xCA
#define ATA_COMMAND_ID_ATA      0xEC

#define ATA_DATA_FEATURE 0x01
#define ATA_DATA_COUNT0  0x02
#define ATA_DATA_LBA0    0x03
#define ATA_DATA_LBA1    0x04
#define ATA_DATA_LBA2    0x05
#define ATA_DATA_SELECT  0x06
#define ATA_DATA_COMMAND 0x07
#define ATA_DATA_COUNT1  0x08
#define ATA_DATA_LBA3    0x09
#define ATA_DATA_LBA4    0x0A
#define ATA_DATA_LBA5    0x0B

#define ATA_ID_TYPE     0x00
#define ATA_ID_SERIAL   0x0A
#define ATA_ID_MODEL    0x1B
#define ATA_ID_CAP      0x31
#define ATA_ID_VALID    0x35
#define ATA_ID_LBA28MAX 0x3C
#define ATA_ID_SUPPORT  0x53
#define ATA_ID_LBA48MAX 0x64

#define ATA_PRIMARY_MASTER_DATA      0x1F0
#define ATA_PRIMARY_MASTER_CONTROL   0x3F6
#define ATA_PRIMARY_SLAVE_DATA       0x170
#define ATA_PRIMARY_SLAVE_CONTROL    0x376
#define ATA_SECONDARY_MASTER_DATA    0x1E8
#define ATA_SECONDARY_MASTER_CONTROL 0x3E6
#define ATA_SECONDARY_SLAVE_DATA     0x168
#define ATA_SECONDARY_SLAVE_CONTROL  0x366

#define ATA_STATUS_FLAG_ERROR 1 << 0
#define ATA_STATUS_FLAG_DRQ   1 << 3
#define ATA_STATUS_FLAG_SRV   1 << 4
#define ATA_STATUS_FLAG_DF    1 << 5
#define ATA_STATUS_FLAG_RDY   1 << 6
#define ATA_STATUS_FLAG_BUSY  1 << 7

#define ATA_DEVICE_TYPE_UNKNOWN 0x00
#define ATA_DEVICE_TYPE_ATA     0x01
#define ATA_DEVICE_TYPE_ATAPI   0x02
#define ATA_DEVICE_TYPE_SATA    0x03
#define ATA_DEVICE_TYPE_SATAPI  0x04

#define ATA_BUS_TYPE    0x0002
#define ATA_DEVICE_TYPE 0x0004

struct ata_bus;

struct ata_device
{

    struct modules_device base;
    struct ata_bus *bus;
    struct pci_device *pciDevice;
    unsigned int type;
    unsigned int slave;
    unsigned char model[41];
    void (*configure_ata)(struct ata_device *self);
    void (*configure_atapi)(struct ata_device *self);
    unsigned int lba28Max;
    unsigned int (*read_lba28)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    unsigned int (*write_lba28)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    unsigned int lba48MaxLow;
    unsigned int lba48MaxHigh;
    unsigned int (*read_lba48)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
    unsigned int (*write_lba48)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);

};

struct ata_bus
{

    struct modules_bus base;
    unsigned short control;
    unsigned short data;
    unsigned int irq;
    void (*sleep)(struct ata_bus *self);
    void (*wait)(struct ata_bus *self);
    void (*select)(struct ata_bus *self, unsigned char operation, unsigned int slave);
    void (*set_lba)(struct ata_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2);
    void (*set_lba2)(struct ata_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5);
    void (*set_command)(struct ata_bus *self, unsigned char command);
    unsigned int (*detect)(struct ata_bus *self, unsigned int slave);
    unsigned int (*read_blocks)(struct ata_bus *self, unsigned int count, void *buffer);
    unsigned int (*write_blocks)(struct ata_bus *self, unsigned int count, void *buffer);
    void (*scan)(struct ata_bus *self, void (*callback)(struct ata_bus *bus, unsigned int slave, unsigned int type));

};

extern void ata_device_init(struct ata_device *device, struct ata_bus *bus, unsigned int slave, unsigned int type);
extern void ata_bus_init(struct ata_bus *bus, unsigned int control, unsigned int data, unsigned int irq);

#endif

