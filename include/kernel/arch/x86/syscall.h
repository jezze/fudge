#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

#define SYSCALL_ROUTINES_SIZE   256
#define SYSCALL_ROUTINE_OPEN    0x00
#define SYSCALL_ROUTINE_CLOSE   0x01
#define SYSCALL_ROUTINE_READ    0x02
#define SYSCALL_ROUTINE_WRITE   0x03
#define SYSCALL_ROUTINE_INFO    0x04
#define SYSCALL_ROUTINE_EXECUTE 0x10
#define SYSCALL_ROUTINE_EXIT    0x11
#define SYSCALL_ROUTINE_WAIT    0x12
#define SYSCALL_ROUTINE_LOAD    0x16
#define SYSCALL_ROUTINE_UNLOAD  0x17
#define SYSCALL_ROUTINE_HALT    0x20
#define SYSCALL_ROUTINE_REBOOT  0x21
#define SYSCALL_ROUTINE_ATTACH  0x30
#define SYSCALL_ROUTINE_DETACH  0x31

struct syscall_registers
{

    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int eip, cs, eflags, useresp, ss;

};

extern void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers));
extern void syscall_unregister_handler(unsigned char index);
extern void syscall_handler(struct syscall_registers *registers);
extern void syscall_init();

#endif

