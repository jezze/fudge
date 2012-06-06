#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/runtime.h>

static struct event_routine routines[EVENT_TABLE_SLOTS];

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, unsigned int callback)
{

    if (routines[index].task)
        return 0;

    event_routine_init(&routines[index], task, callback);

    return 1;

}

unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task)
{

    if (routines[index].task != task)
        return 0;

    event_routine_init(&routines[index], 0, 0);

    return 1;

}

struct runtime_task *event_raise(unsigned int index, struct runtime_task *task)
{

    if (!routines[index].callback)
        return task;

    if (routines[index].task->event)
        return task;

    routines[index].task->event = 1;
    routines[index].task->parentid = task->id;

    runtime_registers_init(&routines[index].task->registers, routines[index].callback, routines[index].task->memory.vaddress + routines[index].task->memory.size, routines[index].task->memory.vaddress + routines[index].task->memory.size);
    runtime_set_running_task(routines[index].task);

    return routines[index].task;

}

void event_routine_init(struct event_routine *routine, struct runtime_task *task, unsigned int callback)
{

    memory_clear(routine, sizeof (struct event_routine));

    routine->task = task;
    routine->callback = callback;

}

