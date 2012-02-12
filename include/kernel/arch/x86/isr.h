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

void isr00();
void isr01();
void isr02();
void isr03();
void isr04();
void isr05();
void isr06();
void isr07();
void isr08();
void isr09();
void isr0A();
void isr0B();
void isr0C();
void isr0D();
void isr0E();
void isr0F();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr1A();
void isr1B();
void isr1C();
void isr1D();
void isr1E();
void isr1F();
void isr_syscall();
void isr_register_routine(unsigned char index, void (*routine)(struct isr_registers *registers));
void isr_unregister_routine(unsigned char index);
void isr_handle(struct isr_registers *registers);
void isr_init();

#endif

