#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#define IRQ_ROUTINES_SIZE 16

struct isr_registers;

extern void irq00();
extern void irq01();
extern void irq02();
extern void irq03();
extern void irq04();
extern void irq05();
extern void irq06();
extern void irq07();
extern void irq08();
extern void irq09();
extern void irq0A();
extern void irq0B();
extern void irq0C();
extern void irq0D();
extern void irq0E();
extern void irq0F();

extern void irq_register_handler(unsigned char index, void (*handler)(struct isr_registers *registers));
extern void irq_unregister_handler(unsigned char index);
extern void irq_init();

#endif

