#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_TABLE_SLOTS 256

struct event_routine
{

    struct runtime_task *task;
    unsigned int callback;

};

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, unsigned int callback);
unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task);
struct runtime_task *event_raise(unsigned int index, struct runtime_task *task);

#endif

