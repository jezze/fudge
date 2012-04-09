#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_ROUTINE_SLOTS   256

#define EVENT_SYSCALL_OPEN    0x80
#define EVENT_SYSCALL_CLOSE   0x81
#define EVENT_SYSCALL_READ    0x82
#define EVENT_SYSCALL_WRITE   0x83
#define EVENT_SYSCALL_EXECUTE 0x90
#define EVENT_SYSCALL_EXIT    0x91
#define EVENT_SYSCALL_WAIT    0x92
#define EVENT_SYSCALL_LOAD    0x96
#define EVENT_SYSCALL_UNLOAD  0x97
#define EVENT_SYSCALL_HALT    0xA0
#define EVENT_SYSCALL_REBOOT  0xA1
#define EVENT_SYSCALL_ATTACH  0xB0
#define EVENT_SYSCALL_DETACH  0xB1

struct event_routine
{

    struct runtime_task *task;
    void (*callback)();

};

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, void (*callback)());
unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task);
void event_raise(unsigned int index);

#endif

