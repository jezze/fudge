#include <kernel/event.h>

void *eventHandlers[32];

void event_register(unsigned int index, void (*handler)())
{

    eventHandlers[index] = handler;

}

void event_unregister(unsigned int index)
{

    eventHandlers[index] = 0;

}

void event_run(unsigned int index)
{

    if (!eventHandlers[index])
        return;

    void (*handler)() = eventHandlers[index];

    handler();

}

void event_init()
{

}

