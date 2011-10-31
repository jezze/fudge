#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

struct event_event eventHandlers[32];

void event_register(unsigned int index, unsigned int pid, void (*handler)())
{

    if (eventHandlers[index].pid != 0)
        return;

    eventHandlers[index].pid = pid;
    eventHandlers[index].handler = handler;

}

void event_unregister(unsigned int index, unsigned int pid)
{

    if (eventHandlers[index].pid != pid)
        return;

    eventHandlers[index].pid = 0;

}

void event_run(unsigned int index, unsigned int pid)
{

    if (!eventHandlers[index].pid)
        return;

    struct runtime_task *oldtask = runtime_get_task(eventHandlers[index].pid);
    runtime_activate(oldtask);

    void (*handler)() = eventHandlers[index].handler;

    handler();

    struct runtime_task *task = runtime_get_task(pid);
    runtime_activate(task);

}

void event_init()
{

}

