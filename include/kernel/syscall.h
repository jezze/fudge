#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

typedef struct syscall_registers
{

    unsigned int eax, ebx, ecx, edx, esi, edi;

} syscall_registers_t;

extern void syscall_handler(syscall_registers_t *registers);
extern void syscall_init();

#endif

