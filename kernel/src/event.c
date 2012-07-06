#include <memory.h>
#include <isr.h>
#include <event.h>
#include <runtime.h>

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

void event_raise(struct runtime_task *task, unsigned int index)
{

    if (!routines[index].callback)
        return;

    if (routines[index].task->event)
        return;

    task->wait = 1;

    routines[index].task->event = 1;
    routines[index].task->wait = 0;
    routines[index].task->parentid = task->id;

    runtime_registers_init(&routines[index].task->registers, routines[index].callback, routines[index].task->memory.vaddress + routines[index].task->memory.size, routines[index].task->memory.vaddress + routines[index].task->memory.size);

}

void event_routine_init(struct event_routine *routine, struct runtime_task *task, unsigned int callback)
{

    memory_clear(routine, sizeof (struct event_routine));

    routine->task = task;
    routine->callback = callback;

}

