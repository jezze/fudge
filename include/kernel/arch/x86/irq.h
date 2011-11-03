#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#define IRQ_ROUTINE_SLOTS 16
#define IRQ_ROUTINE_PIT   0x00
#define IRQ_ROUTINE_KBD   0x01
#define IRQ_ROUTINE_COM2  0x03
#define IRQ_ROUTINE_COM1  0x04
#define IRQ_ROUTINE_SDA   0x06
#define IRQ_ROUTINE_RTC   0x08
#define IRQ_ROUTINE_ATAP  0x0E
#define IRQ_ROUTINE_ATAS  0x0F

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

extern void irq_register_handler(unsigned char index, void (*handler)());
extern void irq_unregister_handler(unsigned char index);
extern void irq_init();

#endif

