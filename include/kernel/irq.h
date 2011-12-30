#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

#define IRQ_ROUTINE_SLOTS 16

struct irq_routine
{

    struct modules_device *device;
    void (*callback)(struct modules_device *device);

};

extern unsigned int irq_register_routine(unsigned char index, struct modules_device *device, void (*callback)(struct modules_device *device));
extern unsigned int irq_unregister_routine(unsigned char index, struct modules_device *device);
extern void irq_raise(unsigned char index);

#endif

