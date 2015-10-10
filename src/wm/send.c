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

    setheader(&wmmap.header, destination, EVENT_WMMAP, 0);
    send(descriptor, sizeof (struct event_wmmap), &wmmap);

}

void send_wmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmunmap wmunmap;

    setheader(&wmunmap.header, destination, EVENT_WMUNMAP, 0);
    send(descriptor, sizeof (struct event_wmunmap), &wmunmap);

}

void send_wmready(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmready wmready;

    setheader(&wmready.header, destination, EVENT_WMREADY, 16);

    wmready.x = x;
    wmready.y = y;
    wmready.w = w;
    wmready.h = h;

    send(descriptor, sizeof (struct event_wmready), &wmready);

}

void send_wmexpose(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmexpose wmexpose;

    setheader(&wmexpose.header, destination, EVENT_WMEXPOSE, 16);

    wmexpose.x = x;
    wmexpose.y = y;
    wmexpose.w = w;
    wmexpose.h = h;

    send(descriptor, sizeof (struct event_wmexpose), &wmexpose);

}

