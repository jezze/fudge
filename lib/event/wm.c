#include <abi.h>
#include <fudge.h>
#include "base.h"
#include "wm.h"

unsigned int event_sendwmresize(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct {struct event_header header; struct event_wmresize wmresize;} message;

    message.wmresize.x = x;
    message.wmresize.y = y;
    message.wmresize.w = w;
    message.wmresize.h = h;
    message.wmresize.padding = padding;
    message.wmresize.lineheight = lineheight;

    return event_send(descriptor, &message.header, EVENT_WMRESIZE, source, target, sizeof (struct event_wmresize));

}

unsigned int event_sendwmshow(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_WMSHOW, source, target, 0);

}

unsigned int event_sendwmhide(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_WMHIDE, source, target, 0);

}

unsigned int event_sendwmflush(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int count, void *buffer)
{

    struct {struct event_header header; char data[0x800];} message;

    return event_send(descriptor, &message.header, EVENT_WMFLUSH, source, target, memory_write(message.data, 0x800, buffer, count, 0));

}

unsigned int event_sendwmkeypress(unsigned int descriptor, unsigned int source, unsigned int target, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeypress wmkeypress;} message;

    message.wmkeypress.scancode = scancode;

    return event_send(descriptor, &message.header, EVENT_WMKEYPRESS, source, target, sizeof (struct event_wmkeypress));

}

unsigned int event_sendwmkeyrelease(unsigned int descriptor, unsigned int source, unsigned int target, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeyrelease wmkeyrelease;} message;

    message.wmkeyrelease.scancode = scancode;

    return event_send(descriptor, &message.header, EVENT_WMKEYRELEASE, source, target, sizeof (struct event_wmkeyrelease));

}

unsigned int event_sendwmmousemove(unsigned int descriptor, unsigned int source, unsigned int target, char relx, char rely)
{

    struct {struct event_header header; struct event_wmmousemove wmmousemove;} message;

    message.wmmousemove.relx = relx;
    message.wmmousemove.rely = rely;

    return event_send(descriptor, &message.header, EVENT_WMMOUSEMOVE, source, target, sizeof (struct event_wmmousemove));

}

unsigned int event_sendwmmousepress(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int button)
{

    struct {struct event_header header; struct event_wmmousepress wmmousepress;} message;

    message.wmmousepress.button = button;

    return event_send(descriptor, &message.header, EVENT_WMMOUSEPRESS, source, target, sizeof (struct event_wmmousepress));

}

unsigned int event_sendwmmouserelease(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int button)
{

    struct {struct event_header header; struct event_wmmouserelease wmmouserelease;} message;

    message.wmmouserelease.button = button;

    return event_send(descriptor, &message.header, EVENT_WMMOUSERELEASE, source, target, sizeof (struct event_wmmouserelease));

}

