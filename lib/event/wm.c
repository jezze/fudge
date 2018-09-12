#include <abi.h>
#include <fudge.h>
#include "base.h"
#include "wm.h"

void event_sendwmmap(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_WMMAP, 0);

}

void event_sendwmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_WMUNMAP, 0);

}

void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event event;
    struct event_wmresize *wmresize = (struct event_wmresize *)event.data;

    wmresize->x = x;
    wmresize->y = y;
    wmresize->w = w;
    wmresize->h = h;
    wmresize->padding = padding;
    wmresize->lineheight = lineheight;

    event_send(descriptor, &event, destination, EVENT_WMRESIZE, sizeof (struct event_wmresize));

}

void event_sendwmshow(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_WMSHOW, 0);

}

void event_sendwmhide(unsigned int descriptor, unsigned int destination)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_WMHIDE, 0);

}

void event_sendwmflush(unsigned int descriptor, unsigned int destination, unsigned int count, void *buffer)
{

    struct event event;

    event_send(descriptor, &event, destination, EVENT_WMFLUSH, memory_write(event.data, FUDGE_BSIZE, buffer, count, 0));

}

void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event event;
    struct event_wmkeypress *wmkeypress = (struct event_wmkeypress *)event.data;

    wmkeypress->scancode = scancode;

    event_send(descriptor, &event, destination, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress));

}

void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event event;
    struct event_wmkeyrelease *wmkeyrelease = (struct event_wmkeyrelease *)event.data;

    wmkeyrelease->scancode = scancode;

    event_send(descriptor, &event, destination, EVENT_WMKEYRELEASE, sizeof (struct event_wmkeyrelease));

}

void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely)
{

    struct event event;
    struct event_wmmousemove *wmmousemove = (struct event_wmmousemove *)event.data;

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    event_send(descriptor, &event, destination, EVENT_WMMOUSEMOVE, sizeof (struct event_wmmousemove));

}

void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct event event;
    struct event_wmmousepress *wmmousepress = (struct event_wmmousepress *)event.data;

    wmmousepress->button = button;

    event_send(descriptor, &event, destination, EVENT_WMMOUSEPRESS, sizeof (struct event_wmmousepress));

}

void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct event event;
    struct event_wmmouserelease *wmmouserelease = (struct event_wmmouserelease *)event.data;

    wmmouserelease->button = button;

    event_send(descriptor, &event, destination, EVENT_WMMOUSERELEASE, sizeof (struct event_wmmouserelease));

}

