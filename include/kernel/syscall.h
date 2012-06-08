#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_TABLE_SLOTS   16
#define SYSCALL_INDEX_OPEN    0x00
#define SYSCALL_INDEX_CLOSE   0x01
#define SYSCALL_INDEX_READ    0x02
#define SYSCALL_INDEX_WRITE   0x03
#define SYSCALL_INDEX_EXECUTE 0x04
#define SYSCALL_INDEX_EXIT    0x05
#define SYSCALL_INDEX_WAIT    0x06
#define SYSCALL_INDEX_LOAD    0x07
#define SYSCALL_INDEX_UNLOAD  0x08
#define SYSCALL_INDEX_HALT    0x09
#define SYSCALL_INDEX_REBOOT  0x0A
#define SYSCALL_INDEX_ATTACH  0x0B
#define SYSCALL_INDEX_DETACH  0x0C

unsigned int syscall_attach(struct kernel_context *context, unsigned int index, unsigned int callback);
unsigned int syscall_close(struct kernel_context *context, unsigned int index);
unsigned int syscall_detach(struct kernel_context *context, unsigned int index);
unsigned int syscall_halt(struct kernel_context *context);
unsigned int syscall_execute(struct kernel_context *context, unsigned int index);
unsigned int syscall_exit(struct kernel_context *context);
unsigned int syscall_load(struct kernel_context *context, unsigned int index);
unsigned int syscall_open(struct kernel_context *context, unsigned int index, char *path);
unsigned int syscall_read(struct kernel_context *context, unsigned int index, unsigned int offset, unsigned int count, char *buffer);
unsigned int syscall_reboot(struct kernel_context *context);
unsigned int syscall_unload(struct kernel_context *context, unsigned int index);
unsigned int syscall_wait(struct kernel_context *context);
unsigned int syscall_write(struct kernel_context *context, unsigned int index, unsigned int offset, unsigned int count, char *buffer);

#endif

