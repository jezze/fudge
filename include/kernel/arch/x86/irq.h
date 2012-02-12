#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#define IRQ_ROUTINE_PIT   0x00
#define IRQ_ROUTINE_KBD   0x01
#define IRQ_ROUTINE_COM2  0x03
#define IRQ_ROUTINE_COM1  0x04
#define IRQ_ROUTINE_SDA   0x06
#define IRQ_ROUTINE_RTC   0x08
#define IRQ_ROUTINE_ATAP  0x0E
#define IRQ_ROUTINE_ATAS  0x0F

struct irq_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int index, slave;
    unsigned int eip, cs, eflags, useresp, ss;

};

void irq00();
void irq01();
void irq02();
void irq03();
void irq04();
void irq05();
void irq06();
void irq07();
void irq08();
void irq09();
void irq0A();
void irq0B();
void irq0C();
void irq0D();
void irq0E();
void irq0F();

void irq_handle(struct irq_registers *registers);
void irq_init();

#endif

