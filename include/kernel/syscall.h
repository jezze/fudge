#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

typedef struct syscall_registers
{

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

} syscall_registers_t;

extern void syscall_handler(syscall_registers_t *registers);
extern void syscall_init();

#endif

