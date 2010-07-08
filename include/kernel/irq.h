#ifndef IRQ_H
#define IRQ_H

#define IRQ_ROUTINES_SIZE 16

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void irq_register_handler(unsigned char index, void (*handler)(registers_t *registers));
extern void irq_unregister_handler(unsigned char index);
extern void irq_init();

#endif

