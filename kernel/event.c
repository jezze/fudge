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

struct event_event *event_get(unsigned int index)
{

    return &eventHandlers[index];

}

void event_init()
{

}

