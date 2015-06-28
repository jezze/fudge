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

void send_wmresize(unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmresize wmresize;

    wmresize.header.source = 0;
    wmresize.header.destination = destination;
    wmresize.header.type = EVENT_WMRESIZE;
    wmresize.header.count = sizeof (struct event_wmresize) - sizeof (struct event_header);
    wmresize.x = x;
    wmresize.y = y;
    wmresize.w = w;
    wmresize.h = h;

    send(sizeof (struct event_wmresize), 1, &wmresize);

}

void send_wmdraw(unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmdraw wmdraw;

    wmdraw.header.source = 0;
    wmdraw.header.destination = destination;
    wmdraw.header.type = EVENT_WMDRAW;
    wmdraw.header.count = sizeof (struct event_wmdraw) - sizeof (struct event_header);
    wmdraw.x = x;
    wmdraw.y = y;
    wmdraw.w = w;
    wmdraw.h = h;

    send(sizeof (struct event_wmdraw), 1, &wmdraw);

}

void send_wmquit(unsigned int destination)
{

    struct event_wmquit wmquit;

    wmquit.header.source = 0;
    wmquit.header.destination = destination;
    wmquit.header.type = EVENT_WMQUIT;
    wmquit.header.count = sizeof (struct event_wmquit) - sizeof (struct event_header);

    send(sizeof (struct event_wmquit), 1, &wmquit);

}

