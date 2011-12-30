#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

#define IRQ_ROUTINE_SLOTS 16

extern void irq_register_routine(unsigned char index, void (*routine)());
extern void irq_unregister_routine(unsigned char index);
extern void irq_routine(unsigned char index);

#endif

