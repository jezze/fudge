#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_TABLE_SLOTS   256
#define SYSCALL_INDEX_OPEN    0x00
#define SYSCALL_INDEX_CLOSE   0x01
#define SYSCALL_INDEX_READ    0x02
#define SYSCALL_INDEX_WRITE   0x03
#define SYSCALL_INDEX_EXECUTE 0x10
#define SYSCALL_INDEX_EXIT    0x11
#define SYSCALL_INDEX_WAIT    0x12
#define SYSCALL_INDEX_LOAD    0x16
#define SYSCALL_INDEX_UNLOAD  0x17
#define SYSCALL_INDEX_HALT    0x20
#define SYSCALL_INDEX_REBOOT  0x21
#define SYSCALL_INDEX_ATTACH  0x30
#define SYSCALL_INDEX_DETACH  0x31

struct runtime_task *syscall_execute(char *path);
unsigned int syscall_raise(unsigned int index, struct runtime_task *task);
void syscall_init();

#endif

