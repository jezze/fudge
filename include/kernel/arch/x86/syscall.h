#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

struct syscall_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int eip, cs, eflags, useresp, ss;

};

void syscall_handle(struct syscall_registers *registers);

#endif

