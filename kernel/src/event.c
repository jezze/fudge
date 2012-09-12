#include <memory.h>
#include <event.h>
#include <runtime.h>

static struct event_routine routines[EVENT_TABLE_SLOTS];

unsigned int event_set_routine(unsigned int index, struct runtime_task *task, unsigned int callback)
{

    if (index > EVENT_TABLE_SLOTS)
        return 0;

    if (routines[index].task)
        return 0;

    routines[index].task = task;
    routines[index].callback = callback;

    return 1;

}

unsigned int event_unset_routine(unsigned int index, struct runtime_task *task)
{

    if (index > EVENT_TABLE_SLOTS)
        return 0;

    if (routines[index].task != task)
        return 0;

    routines[index].task = 0;
    routines[index].callback = 0;

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

