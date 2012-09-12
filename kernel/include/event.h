#ifndef KERNEL_EVENT_H
#define KERNEL_EVENT_H

#define EVENT_TABLE_SLOTS 256

struct event_routine
{

    struct runtime_task *task;
    unsigned int callback;

};

unsigned int event_set_routine(unsigned int index, struct runtime_task *task, unsigned int callback);
unsigned int event_unset_routine(unsigned int index, struct runtime_task *task);
void event_raise(unsigned int index);

#endif

