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
void event_raise(struct isr_context *context, unsigned int index);
void event_routine_init(struct event_routine *routine, struct runtime_task *task, unsigned int callback);

#endif

