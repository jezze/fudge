#ifndef IRQ_H
#define IRQ_H

#include <idt.h>

extern void irq_register_handler(uint8_t, void (*)(registers_t *));
extern void irq_unregister_handler(uint8_t);
extern void irq_init();

#endif

