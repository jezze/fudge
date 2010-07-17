#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

struct syscall_registers
{

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

};

extern void syscall_handler(struct syscall_registers *registers);
extern void syscall_init();

#endif

