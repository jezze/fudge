#include <fudge.h>
#include <kernel.h>
#include "event.h"

void event_unicast(struct list *links, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct task *task = current->data;

        if (header->destination != (unsigned int)task)
            continue;

        task_write(task, header, count);

    }

}

void event_multicast(struct list *links, struct event_header *header, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct task *task = current->data;

        header->destination = (unsigned int)task;

        task_write(task, header, count);

    }

}

unsigned int event_send(struct list *links, struct task_descriptor *descriptor, void *buffer, unsigned int count)
{

    struct event_header *header = buffer;

    header->source = (unsigned int)descriptor->link.data;

    if (header->destination)
        event_unicast(links, header, count);
    else
        event_multicast(links, header, count);

    return count;

}

