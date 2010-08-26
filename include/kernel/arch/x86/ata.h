#ifndef ARCH_ATA_H
#define ARCH_ATA_H

#define ATA_CONTROL_STATUS 0x02

#define ATA_COMMAND_IDENTIFY       0xEC
#define ATA_COMMAND_ATAPI          0xA0
#define ATA_COMMAND_ATAPI_EJECT    0x1B
#define ATA_COMMAND_ATAPI_IDENTIFY 0xA1

#define ATA_DATA_COMMAND 0x07
#define ATA_DATA_LBA0    0x03
#define ATA_DATA_LBA1    0x04
#define ATA_DATA_LBA2    0x05
#define ATA_DATA_LBA3    0x09
#define ATA_DATA_LBA4    0x0A
#define ATA_DATA_LBA5    0x0B
#define ATA_DATA_SELECT  0x06

#define ATA_PRIMARY_MASTER_CONTROL 0x3F6
#define ATA_PRIMARY_MASTER_DATA    0x1F0
#define ATA_PRIMARY_SLAVE_CONTROL  0x376
#define ATA_PRIMARY_SLAVE_DATA     0x170

#define ATA_SECONDARY_MASTER_CONTROL 0x3E6
#define ATA_SECONDARY_MASTER_DATA    0x1E8
#define ATA_SECONDARY_SLAVE_CONTROL  0x366
#define ATA_SECONDARY_SLAVE_DATA     0x168

#define ATA_STATUS_ERROR 0x01

#define ATA_DEVICE_TYPE_UNKNOWN 0x00
#define ATA_DEVICE_TYPE_ATA     0x01
#define ATA_DEVICE_TYPE_ATAPI   0x02
#define ATA_DEVICE_TYPE_SATA    0x03
#define ATA_DEVICE_TYPE_SATAPI  0x04

struct ata_device
{

    unsigned short control;
    unsigned short data;
    unsigned char secondary;
    unsigned char type;

};

extern void ata_init();

#endif

