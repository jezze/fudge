#ifndef MODULES_MSR_H
#define MODULES_MSR_H

#define MSR_DRIVER_TYPE 0x1301

struct msr_data
{

    unsigned int low;
    unsigned int high;

};

struct msr_driver
{

    struct modules_driver base;
    void (*get_msr)(unsigned int msr, struct msr_data *data);
    void (*set_msr)(unsigned int msr, struct msr_data *data);

};

void msr_set(unsigned int msr, struct msr_data *data);
void msr_get(unsigned int msr, struct msr_data *data);
void msr_driver_init(struct msr_driver *driver);

#endif

