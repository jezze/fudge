#define ARCH_KERNEL_BASE                0x00000000
#define ARCH_KERNEL_SIZE                0x00400000
#define ARCH_STACK_BASE                 0x00400000

struct arch_mmu_registers
{

    unsigned int ds;
    struct {unsigned int edi; unsigned int esi; unsigned int ebp; unsigned int esp; unsigned int ebx; unsigned int edx; unsigned int ecx; unsigned int eax;} general;
    unsigned int type;
    struct {unsigned int eip; unsigned int cs; unsigned int eflags; unsigned int esp; unsigned int ss;} interrupt;

};

struct arch_syscall_registers
{

    struct {unsigned int edi; unsigned int esi; unsigned int ebp; unsigned int esp; unsigned int ebx; unsigned int edx; unsigned int ecx; unsigned int eax;} general;
    struct {unsigned int eip; unsigned int cs; unsigned int eflags; unsigned int esp; unsigned int ss;} interrupt;

};

struct arch_x86
{

    struct runtime_task *running;
    struct {unsigned short cs0; unsigned short ds0; unsigned short cs3; unsigned short ds3;} segments;

};

void arch_disable_pic();
void arch_halt();
void arch_usermode(unsigned int cs, unsigned int ds, unsigned int ip, unsigned int sp);
void arch_isr_pagefault();
void arch_isr_syscall();
void arch_setup(unsigned int ramdiskc, void **ramdiskv);
