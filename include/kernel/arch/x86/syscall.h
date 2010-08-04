#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

#define SYSCALL_TABLE_SIZE 256

struct syscall_registers
{

    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;

};

extern void syscall_vfs_walk(struct syscall_registers *registers);
extern void syscall_vfs_find(struct syscall_registers *registers);
extern void syscall_reboot(struct syscall_registers *registers);
extern void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers));
extern void syscall_unregister_handler(unsigned char index);
extern void syscall_handler(struct syscall_registers *registers);
extern void syscall_init();

#endif

