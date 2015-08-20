#include <abi.h>
#include <fudge.h>

static void send(unsigned int descriptor, unsigned int size, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 17, "system/event/send");
    call_open(descriptor);
    call_write(descriptor, 0, size, count, buffer);
    call_close(descriptor);

}

static void setheader(struct event_header *header, unsigned int destination, unsigned int type, unsigned int count)
{

    header->source = 0;
    header->destination = destination;
    header->type = type;
    header->count = count;

}

void send_wmmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmmap wmmap;

    setheader(&wmmap.header, destination, EVENT_WMMAP, 0);
    send(descriptor, sizeof (struct event_wmmap), 1, &wmmap);

}

void send_wmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event_wmunmap wmunmap;

    setheader(&wmunmap.header, destination, EVENT_WMUNMAP, 0);
    send(descriptor, sizeof (struct event_wmunmap), 1, &wmunmap);

}

void send_wmready(unsigned int descriptor, unsigned int destination)
{

    struct event_wmready wmready;

    setheader(&wmready.header, destination, EVENT_WMREADY, 0);
    send(descriptor, sizeof (struct event_wmready), 1, &wmready);

}

void send_wmadd(unsigned int descriptor, unsigned int destination, unsigned int count, void *data)
{

    struct event_wmadd wmadd;

    setheader(&wmadd.header, destination, EVENT_WMADD, count);
    memory_copy(wmadd.data, data, count);
    send(descriptor, sizeof (struct event_header) + count, 1, &wmadd);

}

