#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

#define IRQ_ROUTINE_SLOTS 16

struct runtime_task;

struct irq_routine
{

    struct modules_device *device;
    void (*callback)(struct modules_device *device);

};

unsigned int irq_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device));
unsigned int irq_unregister_routine(unsigned int index, struct modules_device *device);
void irq_raise(unsigned int index, struct runtime_task *task);

#endif

