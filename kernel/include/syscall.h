#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#define SYSCALL_TABLE_SLOTS   16
#define SYSCALL_INDEX_NULL    0x00
#define SYSCALL_INDEX_OPEN    0x01
#define SYSCALL_INDEX_CLOSE   0x02
#define SYSCALL_INDEX_READ    0x03
#define SYSCALL_INDEX_WRITE   0x04
#define SYSCALL_INDEX_EXECUTE 0x05
#define SYSCALL_INDEX_EXIT    0x06
#define SYSCALL_INDEX_WAIT    0x07
#define SYSCALL_INDEX_LOAD    0x08
#define SYSCALL_INDEX_UNLOAD  0x09
#define SYSCALL_INDEX_ATTACH  0x0A
#define SYSCALL_INDEX_DETACH  0x0B

unsigned int syscall_attach(struct runtime_task *task, unsigned int index, unsigned int callback);
unsigned int syscall_close(struct runtime_task *task, unsigned int index);
unsigned int syscall_detach(struct runtime_task *task, unsigned int index);
unsigned int syscall_execute(struct runtime_task *task, unsigned int index);
unsigned int syscall_exit(struct runtime_task *task);
unsigned int syscall_load(struct runtime_task *task, unsigned int index);
unsigned int syscall_open(struct runtime_task *task, unsigned int index, void *buffer);
unsigned int syscall_read(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, void *buffer);
unsigned int syscall_unload(struct runtime_task *task, unsigned int index);
unsigned int syscall_wait(struct runtime_task *task);
unsigned int syscall_write(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, void *buffer);

#endif

