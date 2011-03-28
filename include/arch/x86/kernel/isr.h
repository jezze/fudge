#ifndef ARCH_ISR_H
#define ARCH_ISR_H

#define ISR_ROUTINES_SIZE                      256
#define ISR_ROUTINE_DIVIDEBYZERO               0x00
#define ISR_ROUTINE_DEBUG                      0x01
#define ISR_ROUTINE_NONMASKABLEINTERRUPT       0x02
#define ISR_ROUTINE_BREAKPOINT                 0x03
#define ISR_ROUTINE_INTODETECTED               0x04
#define ISR_ROUTINE_OUTOFBOUNDS                0x05
#define ISR_ROUTINE_INVALIDOPCODE              0x06
#define ISR_ROUTINE_NOCOPROCESSOR              0x07
#define ISR_ROUTINE_DOUBLEFAULT                0x08
#define ISR_ROUTINE_COPROCESSORSEGMENTOVERRUN  0x09
#define ISR_ROUTINE_TSS                        0x0A
#define ISR_ROUTINE_NOSEGMENT                  0x0B
#define ISR_ROUTINE_STACKFAULT                 0x0C
#define ISR_ROUTINE_GENERALPROTECTIONFAULT     0x0D
#define ISR_ROUTINE_PAGEFAULT                  0x0E
#define ISR_ROUTINE_UNKNOWNINTERRUPT           0x0F
#define ISR_ROUTINE_COPROCESSORFAULT           0x10
#define ISR_ROUTINE_ALIGNMENTCHECK             0x11
#define ISR_ROUTINE_MACHINECHECK               0x12
#define ISR_ROUTINE_RESERVED13                 0x13
#define ISR_ROUTINE_RESERVED14                 0x14
#define ISR_ROUTINE_RESERVED15                 0x15
#define ISR_ROUTINE_RESERVED16                 0x16
#define ISR_ROUTINE_RESERVED17                 0x17
#define ISR_ROUTINE_RESERVED18                 0x18
#define ISR_ROUTINE_RESERVED19                 0x19
#define ISR_ROUTINE_RESERVED1A                 0x1A
#define ISR_ROUTINE_RESERVED1B                 0x1B
#define ISR_ROUTINE_RESERVED1C                 0x1C
#define ISR_ROUTINE_RESERVED1D                 0x1D
#define ISR_ROUTINE_RESERVED1E                 0x1E
#define ISR_ROUTINE_RESERVED1F                 0x1F
#define ISR_ROUTINE_SYSCALL                    0x80

struct isr_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int number, error;
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

