#ifndef ARCH_ISR_H
#define ARCH_ISR_H

#define ISR_TABLE_SLOTS   256
#define ISR_INDEX_DE      0x00
#define ISR_INDEX_DB      0x01
#define ISR_INDEX_NI      0x02
#define ISR_INDEX_BP      0x03
#define ISR_INDEX_OF      0x04
#define ISR_INDEX_BR      0x05
#define ISR_INDEX_UD      0x06
#define ISR_INDEX_NM      0x07
#define ISR_INDEX_DF      0x08
#define ISR_INDEX_CO      0x09
#define ISR_INDEX_TS      0x0A
#define ISR_INDEX_NP      0x0B
#define ISR_INDEX_SS      0x0C
#define ISR_INDEX_GP      0x0D
#define ISR_INDEX_PF      0x0E
#define ISR_INDEX_MF      0x10
#define ISR_INDEX_AC      0x11
#define ISR_INDEX_MC      0x12
#define ISR_INDEX_XM      0x13

struct isr_general_registers
{

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

};

struct isr_interrupt_registers
{

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;

};

struct isr_registers
{

    struct isr_general_registers general;
    unsigned int index;
    unsigned int extra;
    struct isr_interrupt_registers interrupt;

};

unsigned int isr_handle(struct isr_registers *registers);
void isr_set_routine(unsigned int index, void (*routine)(struct isr_registers *registers));
void isr_unset_routine(unsigned int index);
void isr_raise(unsigned int index, struct isr_registers *registers);
void isr_setup(unsigned int cs);

#endif

