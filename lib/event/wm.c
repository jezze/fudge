#include <abi.h>
#include <fudge.h>
#include "base.h"
#include "wm.h"

void event_sendwmmap(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMMAP, &message, sizeof (message));

}

void event_sendwmunmap(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMUNMAP, &message, sizeof (message));

}

void event_sendwminit(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMINIT, &message, sizeof (message));

}

void event_sendwmexit(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMEXIT, &message, sizeof (message));

}

void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct {struct event_header header; struct event_wmresize wmresize;} message;

    message.wmresize.x = x;
    message.wmresize.y = y;
    message.wmresize.w = w;
    message.wmresize.h = h;
    message.wmresize.padding = padding;
    message.wmresize.lineheight = lineheight;

    event_send(descriptor, destination, EVENT_WMRESIZE, &message, sizeof (message));

}

void event_sendwmshow(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMSHOW, &message, sizeof (message));

}

void event_sendwmhide(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMHIDE, &message, sizeof (message));

}

void event_sendwmflush(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, destination, EVENT_WMFLUSH, &message, sizeof (message));

}

void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeypress wmkeypress;} message;

    message.wmkeypress.scancode = scancode;

    event_send(descriptor, destination, EVENT_WMKEYPRESS, &message, sizeof (message));

}

void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeyrelease wmkeyrelease;} message;

    message.wmkeyrelease.scancode = scancode;

    event_send(descriptor, destination, EVENT_WMKEYRELEASE, &message, sizeof (message));

}

void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely)
{

    struct {struct event_header header; struct event_wmmousemove wmmousemove;} message;

    message.wmmousemove.relx = relx;
    message.wmmousemove.rely = rely;

    event_send(descriptor, destination, EVENT_WMMOUSEMOVE, &message, sizeof (message));

}

void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmousepress wmmousepress;} message;

    message.wmmousepress.button = button;

    event_send(descriptor, destination, EVENT_WMMOUSEPRESS, &message, sizeof (message));

}

void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmouserelease wmmouserelease;} message;

    message.wmmouserelease.button = button;

    event_send(descriptor, destination, EVENT_WMMOUSERELEASE, &message, sizeof (message));

}

