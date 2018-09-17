#include <abi.h>
#include <fudge.h>
#include "base.h"
#include "wm.h"

void event_sendwmmap(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_WMMAP, source, destination, 0);

}

void event_sendwmunmap(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_WMUNMAP, source, destination, 0);

}

void event_sendwmresize(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct {struct event_header header; struct event_wmresize wmresize;} message;

    message.wmresize.x = x;
    message.wmresize.y = y;
    message.wmresize.w = w;
    message.wmresize.h = h;
    message.wmresize.padding = padding;
    message.wmresize.lineheight = lineheight;

    event_send(descriptor, &message.header, EVENT_WMRESIZE, source, destination, sizeof (struct event_wmresize));

}

void event_sendwmshow(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_WMSHOW, source, destination, 0);

}

void event_sendwmhide(unsigned int descriptor, unsigned int source, unsigned int destination)
{

    struct {struct event_header header;} message;

    event_send(descriptor, &message.header, EVENT_WMHIDE, source, destination, 0);

}

void event_sendwmflush(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int count, void *buffer)
{

    struct {struct event_header header; char data[FUDGE_BSIZE];} message;

    event_send(descriptor, &message.header, EVENT_WMFLUSH, source, destination, memory_write(message.data, FUDGE_BSIZE, buffer, count, 0));

}

void event_sendwmkeypress(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeypress wmkeypress;} message;

    message.wmkeypress.scancode = scancode;

    event_send(descriptor, &message.header, EVENT_WMKEYPRESS, source, destination, sizeof (struct event_wmkeypress));

}

void event_sendwmkeyrelease(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeyrelease wmkeyrelease;} message;

    message.wmkeyrelease.scancode = scancode;

    event_send(descriptor, &message.header, EVENT_WMKEYRELEASE, source, destination, sizeof (struct event_wmkeyrelease));

}

void event_sendwmmousemove(unsigned int descriptor, unsigned int source, unsigned int destination, char relx, char rely)
{

    struct {struct event_header header; struct event_wmmousemove wmmousemove;} message;

    message.wmmousemove.relx = relx;
    message.wmmousemove.rely = rely;

    event_send(descriptor, &message.header, EVENT_WMMOUSEMOVE, source, destination, sizeof (struct event_wmmousemove));

}

void event_sendwmmousepress(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmousepress wmmousepress;} message;

    message.wmmousepress.button = button;

    event_send(descriptor, &message.header, EVENT_WMMOUSEPRESS, source, destination, sizeof (struct event_wmmousepress));

}

void event_sendwmmouserelease(unsigned int descriptor, unsigned int source, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmouserelease wmmouserelease;} message;

    message.wmmouserelease.button = button;

    event_send(descriptor, &message.header, EVENT_WMMOUSERELEASE, source, destination, sizeof (struct event_wmmouserelease));

}

