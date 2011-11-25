#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_IRQ_PIT         0x00
#define EVENT_IRQ_KBD         0x01
#define EVENT_IRQ_MOUSE       0x0C

#define EVENT_SYSCALL_CLOSE   0x20
#define EVENT_SYSCALL_EXECUTE 0x21
#define EVENT_SYSCALL_EXIT    0x22
#define EVENT_SYSCALL_OPEN    0x23
#define EVENT_SYSCALL_READ    0x24
#define EVENT_SYSCALL_WAIT    0x25
#define EVENT_SYSCALL_WRITE   0x26
#define EVENT_SYSCALL_LOAD    0x27
#define EVENT_SYSCALL_UNLOAD  0x28

struct event_routine
{

    struct runtime_task *task;
    void (*routine)();

};

extern unsigned int event_register_routine(unsigned int index, struct runtime_task *task, void (*routine)());
extern unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task);
extern unsigned int event_handle(unsigned int index);

#endif

