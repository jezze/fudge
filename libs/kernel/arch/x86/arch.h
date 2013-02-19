#define ARCH_KERNEL_BASE                0x00000000
#define ARCH_KERNEL_SIZE                0x00400000
#define ARCH_STACK_BASE                 0x00400000

struct arch_registers_general
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

struct arch_registers_interrupt
{

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;

};

struct arch_registers_mmu
{

    unsigned int ds;
    struct arch_registers_general general;
    unsigned int type;
    struct arch_registers_interrupt interrupt;

};

struct arch_registers_syscall
{

    struct arch_registers_general general;
    struct arch_registers_interrupt interrupt;

};

struct arch_state
{

    struct runtime_container *container;
    struct {unsigned short kcode; unsigned short kdata; unsigned short ucode; unsigned short udata;} selectors;

};

void arch_disable_pic();
void arch_halt();
void arch_usermode(unsigned int cs, unsigned int ds, unsigned int ip, unsigned int sp);
void arch_isr_pagefault();
void arch_isr_syscall();
void arch_setup(unsigned int ramdiskc, void **ramdiskv);
