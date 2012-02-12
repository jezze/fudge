#ifndef MODULES_SMP_H
#define MODULES_SMP_H

#define SMP_DRIVER_TYPE 0x6401

struct smp_cpu
{

    unsigned int id;
    unsigned int core;
    unsigned int chip;
    unsigned int domain;

};

struct smp_driver
{

    struct modules_driver base;
    struct smp_cpu cpus[32];
    unsigned int count;

};

void smp_driver_init(struct smp_driver *driver);

#endif

