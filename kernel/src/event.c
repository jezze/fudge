#include <memory.h>
#include <event.h>
#include <runtime.h>

static struct event_routine routines[EVENT_TABLE_SLOTS];

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, unsigned int callback)
{

    if (index > EVENT_TABLE_SLOTS)
        return 0;

    if (routines[index].task)
        return 0;

    event_routine_init(&routines[index], task, callback);

    return 1;

}

unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task)
{

    if (index > EVENT_TABLE_SLOTS)
        return 0;

    if (routines[index].task != task)
        return 0;

    event_routine_init(&routines[index], 0, 0);

    return 1;

}

void event_raise(unsigned int index)
{

    if (!routines[index].callback)
        return;

    if (routines[index].task->status.event)
        return;

    routines[index].task->status.event = 1;
    routines[index].task->status.idle = 0;
    routines[index].task->registers.ip = routines[index].callback;
    routines[index].task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    routines[index].task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

void event_routine_init(struct event_routine *routine, struct runtime_task *task, unsigned int callback)
{

    memory_clear(routine, sizeof (struct event_routine));

    routine->task = task;
    routine->callback = callback;

}

