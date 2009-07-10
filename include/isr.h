#ifndef ISR_H
#define ISR_H

#define ISR_ROUTINES_SIZE 256

extern void isr_register_handler(uint8_t num, void (*handler)(registers_t *r));
extern void isr_unregister_handler(uint8_t num);
extern void isr_init();

#endif

