#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/vfs.h>

#define SYSCALL_SCREEN_PUTS     0x01
#define SYSCALL_SCREEN_PUTS_DEC 0x02
#define SYSCALL_SCREEN_PUTS_HEX 0x03

#define SYSCALL_VFS_READ  0x10
#define SYSCALL_VFS_WRITE 0x11
#define SYSCALL_VFS_WALK  0x12
#define SYSCALL_VFS_FIND  0x13
#define SYSCALL_VFS_ROOT  0x14

#define SYSCALL_RTC_DATE 0x20

#define SYSCALL_PIT_READ 0x30

extern int call_send(unsigned int num, void *data);
extern int call_puts(char *s);
extern int call_puts_dec(unsigned int n);
extern int call_puts_hex(unsigned int n);
extern vfs_node_t* call_vfs_walk(unsigned int index);

extern unsigned int syscall_handler(uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax);
extern void syscall_init();

#endif

