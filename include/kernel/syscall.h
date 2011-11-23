#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_ROUTINE_SLOTS 256
#define SYSCALL_ROUTINE_OPEN    0x00
#define SYSCALL_ROUTINE_CLOSE   0x01
#define SYSCALL_ROUTINE_READ    0x02
#define SYSCALL_ROUTINE_WRITE   0x03
#define SYSCALL_ROUTINE_EXECUTE 0x10
#define SYSCALL_ROUTINE_EXIT    0x11
#define SYSCALL_ROUTINE_WAIT    0x12
#define SYSCALL_ROUTINE_LOAD    0x16
#define SYSCALL_ROUTINE_UNLOAD  0x17
#define SYSCALL_ROUTINE_HALT    0x20
#define SYSCALL_ROUTINE_REBOOT  0x21
#define SYSCALL_ROUTINE_ATTACH  0x30
#define SYSCALL_ROUTINE_DETACH  0x31

extern unsigned int syscall_attach(unsigned int index, void (*handler)());
extern unsigned int syscall_close(unsigned int fd);
extern unsigned int syscall_detach(unsigned int index);
extern unsigned int syscall_execute(char *path, unsigned int argc, char **argv);
extern unsigned int syscall_exit();
extern unsigned int syscall_load(char *path);
extern unsigned int syscall_open(char *view, char *name);
extern unsigned int syscall_read(unsigned int fd, unsigned int count, char *buffer);
extern unsigned int syscall_reboot();
extern unsigned int syscall_unload();
extern unsigned int syscall_wait();
extern unsigned int syscall_write(unsigned int fd, unsigned int count, char *buffer);

#endif

