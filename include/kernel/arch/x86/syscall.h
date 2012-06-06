#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

struct runtime_task;

unsigned int syscall_raise(unsigned int index, struct runtime_task *task);
void syscall_init();

#endif

