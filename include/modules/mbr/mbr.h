#ifndef MODULES_MBR_H
#define MODULES_MBR_H

#define MBR_PARTITION_SLOTS 4
#define MBR_PARTITION_OFFSET 0x1BE
#define MBR_PARTITION_SIZE 16

#define MBR_DRIVER_TYPE 300

struct mbr_partition
{

    unsigned char boot;
    unsigned char headStart;
    unsigned char sectorStart;
    unsigned char cylinderStart;
    unsigned char systemId;
    unsigned char headEnd;
    unsigned char sectorEnd;
    unsigned char cylinderEnd;
    unsigned int sectorLba;
    unsigned int sectorTotal;

} __attribute__((packed));

struct mbr_driver
{

    struct modules_driver base;
    struct mbr_partition *(*get_partition)(struct mbr_driver *self, unsigned int index);

};

extern void mbr_driver_init(struct mbr_driver *driver);

#endif

