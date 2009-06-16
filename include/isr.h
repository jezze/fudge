#ifndef ISR_H
#define ISR_H

extern void isr_register_handler(uint8_t, void (*)(registers_t *));
extern void isr_unregister_handler(uint8_t);
extern void isr_init();

#endif

