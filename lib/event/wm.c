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

unsigned int event_addwmresize(void *buffer, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_header *header = buffer;
    struct event_wmresize *wmresize = event_getdata(buffer);

    header->length += sizeof (struct event_wmresize);
    wmresize->x = x;
    wmresize->y = y;
    wmresize->w = w;
    wmresize->h = h;
    wmresize->padding = padding;
    wmresize->lineheight = lineheight;

    return header->length;

}

unsigned int event_addwmflush(void *buffer, unsigned int count, void *data)
{

    struct event_header *header = buffer;

    header->length += memory_write(buffer, FUDGE_BSIZE, data, count, sizeof (struct event_header));

    return header->length;

}

