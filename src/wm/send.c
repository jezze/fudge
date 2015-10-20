#include <abi.h>
#include <fudge.h>

static void send(unsigned int descriptor, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 17, "system/event/send");
    call_open(descriptor);
    call_write(descriptor, count, buffer);
    call_close(descriptor);

}

static void setheader(struct event_header *header, unsigned int destination, unsigned int type, unsigned int count)
{

    header->destination = destination;
    header->type = type;
    header->count = count;

}

void send_wmmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmmap wmmap;

    setheader(&wmmap.header, destination, EVENT_WMMAP, sizeof (struct event_wmmap) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmmap), &wmmap);

}

void send_wmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmunmap wmunmap;

    setheader(&wmunmap.header, destination, EVENT_WMUNMAP, sizeof (struct event_wmunmap) - sizeof (struct event_header));
    send(descriptor, sizeof (struct event_wmunmap), &wmunmap);

}

void send_wmmapnotify(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmmapnotify wmmapnotify;

    setheader(&wmmapnotify.header, destination, EVENT_WMMAPNOTIFY, sizeof (struct event_wmmapnotify) - sizeof (struct event_header));

    wmmapnotify.x = x;
    wmmapnotify.y = y;
    wmmapnotify.w = w;
    wmmapnotify.h = h;

    send(descriptor, sizeof (struct event_wmmapnotify), &wmmapnotify);

}

void send_wmexpose(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmexpose wmexpose;

    setheader(&wmexpose.header, destination, EVENT_WMEXPOSE, sizeof (struct event_wmexpose) - sizeof (struct event_header));

    wmexpose.x = x;
    wmexpose.y = y;
    wmexpose.w = w;
    wmexpose.h = h;

    send(descriptor, sizeof (struct event_wmexpose), &wmexpose);

}

