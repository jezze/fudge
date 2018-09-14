#include <abi.h>
#include <fudge.h>
#include "base.h"

void event_read(unsigned int descriptor, struct event *event)
{

    while (file_readall(descriptor, &event->header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(descriptor, event->data, event->header.length - sizeof (struct event_header)) != event->header.length - sizeof (struct event_header));

}

void event_send(unsigned int descriptor, struct event *event, unsigned int destination, unsigned int type, unsigned int length)
{

    event->header.destination = destination;
    event->header.type = type;
    event->header.length = sizeof (struct event_header) + length;

    file_writeall(descriptor, event, event->header.length);

}

void event_sendinit(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_INIT, 0);

}

void event_sendexit(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_EXIT, 0);

}

void event_sendkill(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_KILL, 0);

}

void event_sendrein(unsigned int descriptor, unsigned int destination, unsigned char num)
{

    struct event event;
    struct event_rein *rein = (struct event_rein *)event.data;

    rein->num = num;

    event_send(descriptor, &event, destination, EVENT_REIN, sizeof (struct event_rein));

}

void event_sendreout(unsigned int descriptor, unsigned int destination, unsigned char num)
{

    struct event event;
    struct event_reout *reout = (struct event_reout *)event.data;

    reout->num = num;

    event_send(descriptor, &event, destination, EVENT_REOUT, sizeof (struct event_reout));

}

void event_senddata(unsigned int descriptor, unsigned int destination, unsigned int count, void *buffer)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_DATA, memory_read(event.data, FUDGE_BSIZE, buffer, count, 0));

}

