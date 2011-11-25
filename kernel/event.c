#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

struct event_routine eventRoutines[64];

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, void (*routine)())
{

    if (eventRoutines[index].task)
        return 0;

    eventRoutines[index].task = task;
    eventRoutines[index].routine = routine;

    return 1;

}

unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task)
{

    if (eventRoutines[index].task != task)
        return 0;

    eventRoutines[index].task = 0;
    eventRoutines[index].routine = 0;

    return 1;

}

unsigned int event_handle(unsigned int index)
{

    struct event_routine *event = &eventRoutines[index];

    if (!event)
        return 0;

    struct runtime_task *ctask = runtime_get_running_task();

    if (!ctask)
        return 0;

    struct runtime_task *etask = runtime_get_task(event->task->id);

    if (!etask)
        return 0;

    if (ctask->id != etask->id)
    {

        runtime_activate(etask);

        etask->parentid = ctask->id;

    }

    etask->registers.ip = event->routine;

    return 1;

}

