#ifndef IRQ_H
#define IRQ_H

#define IRQ_ROUTINES_SIZE 16

extern void irq_register_handler(uint8_t num, void (*handler)(registers_t *r));
extern void irq_unregister_handler(uint8_t num);
extern void irq_init();

#endif

