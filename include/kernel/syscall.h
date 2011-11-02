#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

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

extern void syscall_attach(unsigned int index, void (*handler)());
extern void syscall_close(unsigned int fd);
extern void syscall_detach(unsigned int index);
extern unsigned int syscall_info(unsigned int fd, struct file_info *info);
extern void syscall_load(char *path);
extern int syscall_open(char *path);
extern unsigned int syscall_read(unsigned int fd, unsigned int count, char *buffer);
extern void syscall_reboot();
extern void syscall_unload();
extern unsigned int syscall_write(unsigned int fd, unsigned int count, char *buffer);

#endif

