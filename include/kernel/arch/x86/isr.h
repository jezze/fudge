#ifndef ARCH_ISR_H
#define ARCH_ISR_H

#define ISR_ROUTINE_SLOTS 256
#define ISR_ROUTINE_DE      0x00
#define ISR_ROUTINE_DB      0x01
#define ISR_ROUTINE_NI      0x02
#define ISR_ROUTINE_BP      0x03
#define ISR_ROUTINE_OF      0x04
#define ISR_ROUTINE_BR      0x05
#define ISR_ROUTINE_UD      0x06
#define ISR_ROUTINE_NM      0x07
#define ISR_ROUTINE_DF      0x08
#define ISR_ROUTINE_CO      0x09
#define ISR_ROUTINE_TS      0x0A
#define ISR_ROUTINE_NP      0x0B
#define ISR_ROUTINE_SS      0x0C
#define ISR_ROUTINE_GP      0x0D
#define ISR_ROUTINE_PF      0x0E
#define ISR_ROUTINE_MF      0x10
#define ISR_ROUTINE_AC      0x11
#define ISR_ROUTINE_MC      0x12
#define ISR_ROUTINE_XM      0x13
#define ISR_ROUTINE_SYSCALL 0x80

struct isr_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int index, error;
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
extern void isr_register_handler(unsigned char index, void (*handler)(struct isr_registers *registers));
extern void isr_unregister_handler(unsigned char index);
extern void isr_init();

#endif

