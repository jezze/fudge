#ifndef MODULES_FPU_H
#define MODULES_FPU_H

#define FPU_DRIVER_TYPE 0x1074

struct fpu_driver
{

    struct modules_driver base;

};

void fpu_enable();
void fpu_driver_init(struct fpu_driver *driver);

#endif

