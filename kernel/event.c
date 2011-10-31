#include <kernel/event.h>

void *eventHandlers[32];

void event_attach(unsigned int index, void (*handler)())
{

    eventHandlers[index] = handler;

}

void event_run(unsigned int index)
{

    void (*handler)() = eventHandlers[index];

    handler();

}

void event_init()
{

}

