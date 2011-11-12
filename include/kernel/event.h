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

struct event_event
{

    struct runtime_task *task;
    void (*handler)();

};

extern void event_register(unsigned int index, struct runtime_task *task, void (*handler)());
extern void event_unregister(unsigned int index, struct runtime_task *task);
extern struct event_event *event_get(unsigned int index);
extern unsigned int event_handler(unsigned int index);
extern void event_init();

#endif

