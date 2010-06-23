#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_ROUTINES_SIZE 3

extern int call_send(uint32_t num, void *data);

extern int syscall_write(char *text);
extern void syscall_handler(uint32_t num, void *data);
extern void syscall_init();

#endif

