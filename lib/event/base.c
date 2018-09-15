#include <abi.h>
#include <fudge.h>
#include "base.h"

unsigned int event_read(unsigned int descriptor, struct event *event)
{

    while (file_readall(descriptor, &event->header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(descriptor, event->data, event->header.length - sizeof (struct event_header)) != event->header.length - sizeof (struct event_header));

    return event->header.length;

}

unsigned int event_send(unsigned int descriptor, struct event *event, unsigned int type, unsigned int source, unsigned int destination, unsigned int length)
{

    event_initheader(&event->header, type, source, destination, length);

    return file_writeall(descriptor, event, event->header.length);

}

void event_sendinit(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, EVENT_INIT, source, destination, 0);

}

void event_sendexit(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, EVENT_EXIT, source, destination, 0);

}

void event_sendkill(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, EVENT_KILL, source, destination, 0);

}

void event_senddata(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int count, void *buffer)
{

    struct event event;

    event_send(descriptor, &event, EVENT_DATA, source, destination, memory_read(event.data, FUDGE_BSIZE, buffer, count, 0));

}

void event_sendfile(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char num)
{

    struct event event;
    struct event_file *file = (struct event_file *)event.data;

    file->num = num;

    event_send(descriptor, &event, EVENT_FILE, source, destination, sizeof (struct event_file));

}

void event_sendchild(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, EVENT_CHILD, source, destination, 0);

}

void event_initheader(struct event_header *header, unsigned int type, unsigned int source, unsigned int destination, unsigned int length)
{

    header->type = type;
    header->source = source;
    header->destination = destination;
    header->length = sizeof (struct event_header) + length;

}

