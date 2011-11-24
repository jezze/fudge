#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

struct event_event eventHandlers[64];

unsigned int event_register(unsigned int index, struct runtime_task *task, void (*handler)())
{

    if (eventHandlers[index].task)
        return 0;

    eventHandlers[index].task = task;
    eventHandlers[index].handler = handler;

    return 1;

}

unsigned int event_unregister(unsigned int index, struct runtime_task *task)
{

    if (eventHandlers[index].task != task)
        return 0;

    eventHandlers[index].task = 0;
    eventHandlers[index].handler = 0;

    return 1;

}

struct event_event *event_get(unsigned int index)
{

    return &eventHandlers[index];

}

unsigned int event_handler(unsigned int index)
{

    struct event_event *event = event_get(index);

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

    etask->registers.ip = event->handler;

    return 1;

}

void event_init()
{

}

