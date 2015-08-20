#include <abi.h>
#include <fudge.h>

static void send(unsigned int size, unsigned int count, void *buffer)
{

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, size, count, buffer);
    call_close(CALL_L2);

}

void send_wmmap(unsigned int destination)
{

    struct event_wmmap wmmap;

    wmmap.header.source = 0;
    wmmap.header.destination = destination;
    wmmap.header.type = EVENT_WMMAP;
    wmmap.header.count = sizeof (struct event_wmmap) - sizeof (struct event_header);

    send(sizeof (struct event_wmmap), 1, &wmmap);

}

void send_wmunmap(unsigned int destination)
{

    struct event_wmunmap wmunmap;

    wmunmap.header.source = 0;
    wmunmap.header.destination = destination;
    wmunmap.header.type = EVENT_WMUNMAP;
    wmunmap.header.count = sizeof (struct event_wmunmap) - sizeof (struct event_header);

    send(sizeof (struct event_wmunmap), 1, &wmunmap);

}

void send_wmready(unsigned int destination)
{

    struct event_wmready wmready;

    wmready.header.source = 0;
    wmready.header.destination = destination;
    wmready.header.type = EVENT_WMREADY;
    wmready.header.count = sizeof (struct event_wmready) - sizeof (struct event_header);

    send(sizeof (struct event_wmready), 1, &wmready);

}

void send_wmadd(unsigned int destination, unsigned int count, void *data)
{

    struct event_wmadd wmadd;

    wmadd.header.source = 0;
    wmadd.header.destination = destination;
    wmadd.header.type = EVENT_WMADD;
    wmadd.header.count = sizeof (struct event_wmadd) - sizeof (struct event_header);

    send(sizeof (struct event_wmadd), 1, &wmadd);

}

