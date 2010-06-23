#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_ROUTINES_SIZE 3
#define SYSCALL_STRING_PUTS 0x01

extern int call_send(uint32_t num, void *data);

extern void syscall_handler(uint32_t num, void *data);
extern void syscall_init();

#endif

