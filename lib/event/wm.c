#include <abi.h>
#include <fudge.h>
#include "base.h"
#include "wm.h"

unsigned int event_addwmkeypress(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_wmkeypress *wmkeypress = event_getdata(buffer);

    header->length += sizeof (struct event_wmkeypress);
    wmkeypress->scancode = scancode;

    return header->length;

}

unsigned int event_addwmkeyrelease(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_wmkeyrelease *wmkeyrelease = event_getdata(buffer);

    header->length += sizeof (struct event_wmkeyrelease);
    wmkeyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addwmmousepress(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_wmmousepress *wmmousepress = event_getdata(buffer);

    header->length += sizeof (struct event_wmmousepress);
    wmmousepress->button = button;

    return header->length;

}

unsigned int event_addwmmouserelease(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_wmmouserelease *wmmouserelease = event_getdata(buffer);

    header->length += sizeof (struct event_wmmouserelease);
    wmmouserelease->button = button;

    return header->length;

}

unsigned int event_addwmmousemove(void *buffer, char relx, char rely)
{

    struct event_header *header = buffer;
    struct event_wmmousemove *wmmousemove = event_getdata(buffer);

    header->length += sizeof (struct event_wmmousemove);
    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    return header->length;

}

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

