#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

struct event_event eventHandlers[32];

void event_register(unsigned int index, unsigned int id, void (*handler)())
{

    if (eventHandlers[index].id != 0)
        return;

    eventHandlers[index].id = id;
    eventHandlers[index].handler = handler;

}

void event_unregister(unsigned int index, unsigned int id)
{

    if (eventHandlers[index].id != id)
        return;

    eventHandlers[index].id = 0;

}

struct event_event *event_get(unsigned int index)
{

    return &eventHandlers[index];

}

void event_init()
{

}

