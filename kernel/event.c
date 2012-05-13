#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>

static struct event_routine routines[EVENT_ROUTINE_SLOTS];

unsigned int event_register_routine(unsigned int index, struct runtime_task *task, unsigned int callback)
{

    struct event_routine *routine = &routines[index];

    if (routine->task)
        return 0;

    routine->task = task;
    routine->callback = callback;

    return 1;

}

unsigned int event_unregister_routine(unsigned int index, struct runtime_task *task)
{

    struct event_routine *routine = &routines[index];

    if (routine->task != task)
        return 0;

    routine->task = 0;
    routine->callback = 0;

    return 1;

}

void event_raise(unsigned int index)
{

    struct event_routine *routine = &routines[index];

    if (!routine->callback)
        return;

    struct runtime_task *task = runtime_get_running_task();
    struct runtime_task *etask = runtime_get_task(routine->task->id);

    if (etask->event)
        return;

    etask->event = 1;
    etask->parentid = task->id;
    etask->load(etask, routine->callback);

    mmu_load_memory(etask->id);
    runtime_set_running_task(etask);

}

