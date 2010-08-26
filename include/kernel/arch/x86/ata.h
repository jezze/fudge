#ifndef ARCH_ATA_H
#define ARCH_ATA_H

#define ATA_CONTROL_STATUS 0x02

#define ATA_DATA_SELECT  0x06
#define ATA_DATA_COMMAND 0x07

#define ATA_PRIMARY_MASTER_CONTROL 0x3F6
#define ATA_PRIMARY_MASTER_DATA    0x1F0
#define ATA_PRIMARY_SLAVE_CONTROL  0x376
#define ATA_PRIMARY_SLAVE_DATA     0x170

#define ATA_SECONDARY_MASTER_CONTROL 0x3E6
#define ATA_SECONDARY_MASTER_DATA    0x1E8
#define ATA_SECONDARY_SLAVE_CONTROL  0x366
#define ATA_SECONDARY_SLAVE_DATA     0x168

struct ata_device
{

    unsigned short control;
    unsigned short data;
    unsigned char secondary;

};

extern void ata_init();

#endif

