#ifndef MODULES_MBR_H
#define MODULES_MBR_H

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
    unsigned int sectorRelative;
    unsigned int sectorTotal;

} __attribute__((packed));

struct mbr_driver
{

    struct modules_driver base;

};

extern void mbr_driver_init(struct mbr_driver *driver);

#endif

