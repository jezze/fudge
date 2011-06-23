#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

#define SYSCALL_ROUTINES_SIZE   256
#define SYSCALL_ROUTINE_OPEN    0x00
#define SYSCALL_ROUTINE_CLOSE   0x01
#define SYSCALL_ROUTINE_READ    0x02
#define SYSCALL_ROUTINE_WRITE   0x03
#define SYSCALL_ROUTINE_INFO    0x04
#define SYSCALL_ROUTINE_MAP     0x10
#define SYSCALL_ROUTINE_HALT    0x20
#define SYSCALL_ROUTINE_REBOOT  0x21

struct syscall_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;

};

extern void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers));
extern void syscall_unregister_handler(unsigned char index);
extern void syscall_handler(struct syscall_registers *registers);
extern void syscall_init();

#endif

