#ifndef MODULES_MBR_H
#define MODULES_MBR_H

#define MBR_DRIVER_TYPE 300

struct mbr_header
{

};

struct mbr_driver
{

    struct modules_driver base;

};

extern void mbr_driver_init(struct mbr_driver *driver);

#endif

