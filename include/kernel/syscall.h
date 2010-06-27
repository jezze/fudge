#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_STRING_PUTS     0x01
#define SYSCALL_STRING_PUTS_DEC 0x02
#define SYSCALL_STRING_PUTS_HEX 0x03

#define SYSCALL_VFS_READ  0x10
#define SYSCALL_VFS_WRITE 0x11
#define SYSCALL_VFS_WALK  0x12
#define SYSCALL_VFS_FIND  0x13
#define SYSCALL_VFS_ROOT  0x14

extern int call_send(uint32_t num, void *data);

extern unsigned int syscall_handler(uint32_t num, void *data);
extern void syscall_init();

#endif

