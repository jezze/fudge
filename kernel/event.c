#include <lib/memory.h>
#include <kernel/event.h>

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

//    runtime_activate(task);

    void (*handler)() = eventHandlers[index].handler;

    handler();

}

void event_init()
{

}

