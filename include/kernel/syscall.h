#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

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

