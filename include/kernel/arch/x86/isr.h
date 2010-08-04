#ifndef ARCH_ISR_H
#define ARCH_ISR_H

#define ISR_ROUTINES_SIZE 256

struct isr_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;

};

extern void isr00();
extern void isr01();
extern void isr02();
extern void isr03();
extern void isr04();
extern void isr05();
extern void isr06();
extern void isr07();
extern void isr08();
extern void isr09();
extern void isr0A();
extern void isr0B();
extern void isr0C();
extern void isr0D();
extern void isr0E();
extern void isr0F();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr1A();
extern void isr1B();
extern void isr1C();
extern void isr1D();
extern void isr1E();
extern void isr1F();
extern void isr_syscall();
extern void isr_enable();
extern void isr_disable();

extern void isr_register_handler(unsigned char index, void (*handler)(struct isr_registers *registers));
extern void isr_unregister_handler(unsigned char index);
extern void isr_init();

#endif

