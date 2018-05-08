#include <abi.h>
#include <fudge.h>
#include "event.h"

unsigned int event_read(struct event *event, unsigned int descriptor)
{

    unsigned int count = file_readall(descriptor, &event->header, sizeof (struct event_header));

    if (!count)
        return 0;

    if (event->header.length - count > 0)
        count += file_readall(descriptor, event->data, event->header.length - count);

    return count;

}

void event_sendwmmap(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMAP;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmunmap(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMUNMAP;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwminit(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMINIT;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmexit(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMEXIT;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct {struct event_header header; struct event_wmresize wmresize;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMRESIZE;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmresize);
    message.wmresize.x = x;
    message.wmresize.y = y;
    message.wmresize.w = w;
    message.wmresize.h = h;
    message.wmresize.padding = padding;
    message.wmresize.lineheight = lineheight;

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmshow(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMSHOW;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmhide(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMHIDE;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmflush(unsigned int descriptor, unsigned int destination)
{

    struct {struct event_header header;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMFLUSH;
    message.header.length = sizeof (struct event_header);

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeypress wmkeypress;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMKEYPRESS;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmkeypress);
    message.wmkeypress.scancode = scancode;

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeyrelease wmkeyrelease;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMKEYRELEASE;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmkeyrelease);
    message.wmkeyrelease.scancode = scancode;

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely)
{

    struct {struct event_header header; struct event_wmmousemove wmmousemove;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSEMOVE;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmmousemove);
    message.wmmousemove.relx = relx;
    message.wmmousemove.rely = rely;

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmousepress wmmousepress;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSEPRESS;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmmousepress);
    message.wmmousepress.button = button;

    file_writeall(descriptor, &message, message.header.length);

}

void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmouserelease wmmouserelease;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSERELEASE;
    message.header.length = sizeof (struct event_header) + sizeof (struct event_wmmouserelease);
    message.wmmouserelease.button = button;

    file_writeall(descriptor, &message, message.header.length);

}

