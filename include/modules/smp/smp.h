#ifndef MODULES_SMP_H
#define MODULES_SMP_H

#define SMP_DRIVER_TYPE 0x6401

struct smp_driver
{

    struct modules_driver base;

};

extern void smp_driver_init(struct smp_driver *driver);

#endif

