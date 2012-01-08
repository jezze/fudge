#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

struct syscall_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int eip, cs, eflags, useresp, ss;

};

struct syscall_registers_quick
{

    unsigned int gs, fs, es, ds;
    unsigned int ebp;
    unsigned int eip, cs, eflags, useresp, ss;

};

extern void syscall_handle(struct syscall_registers *registers);
extern void syscall_handle_quick(unsigned int num, void *parameters);
extern void syscall_init();

#endif

