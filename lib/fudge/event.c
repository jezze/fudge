#include "define.h"
#include "memory.h"
#include "event.h"

static void *addpayload(void *buffer, unsigned int length)
{

    struct event_header *header = buffer;
    char *start = buffer;

    start += header->length;
    header->length += length;

    return start;

}

void *event_getdata(void *buffer)
{

    struct event_header *header = buffer;

    return header + 1;

}

unsigned int event_avail(void *buffer)
{

    struct event_header *header = buffer;

    return FUDGE_BSIZE - header->length;

}

unsigned int event_addheader(void *buffer, unsigned int type, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = 0;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->nroutes = 0;

    return header->length;

}

unsigned int event_addroute(void *buffer, unsigned int target)
{

    struct event_header *header = buffer;

    header->routes[header->nroutes] = target;
    header->nroutes++;

    return header->length;

}

unsigned int event_addrequest(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, id);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes; i++)
            event_addroute(buffer, header->routes[i]);

    }

    event_addroute(buffer, header->target);

    return reply->length;

}

unsigned int event_addpipe(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, id);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes; i++)
            event_addroute(buffer, header->routes[i]);

    }

    return reply->length;

}

unsigned int event_addresponse(void *buffer, struct event_header *header, unsigned int type)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->source);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes - 1; i++)
            event_addroute(buffer, header->routes[i]);

        reply->target = header->routes[header->nroutes - 1];

    }

    return reply->length;

}

unsigned int event_adddata(void *buffer, unsigned int session)
{

    struct event_header *header = buffer;
    struct event_data *d = addpayload(buffer, sizeof (struct event_data));

    d->session = session;
    d->count = 0;

    return header->length;

}

unsigned int event_appenddata(void *buffer, unsigned int count, void *data)
{

    struct event_header *header = buffer;
    struct event_data *d = event_getdata(buffer);
    unsigned int c = memory_write(buffer, FUDGE_BSIZE, data, count, header->length);

    d->count += c;
    header->length += c;

    return header->length;

}

unsigned int event_addfile(void *buffer, unsigned int session, unsigned int descriptor)
{

    struct event_header *header = buffer;
    struct event_file *file = addpayload(buffer, sizeof (struct event_file));

    file->session = session;
    file->descriptor = descriptor;

    return header->length;

}

unsigned int event_addkeypress(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_keypress *keypress = addpayload(buffer, sizeof (struct event_keypress));

    keypress->scancode = scancode;

    return header->length;

}

unsigned int event_addkeyrelease(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_keyrelease *keyrelease = addpayload(buffer, sizeof (struct event_keyrelease));

    keyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addmousepress(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_mousepress *mousepress = addpayload(buffer, sizeof (struct event_mousepress));

    mousepress->button = button;

    return header->length;

}

unsigned int event_addmouserelease(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_mouserelease *mouserelease = addpayload(buffer, sizeof (struct event_mouserelease));

    mouserelease->button = button;

    return header->length;

}

unsigned int event_addmousemove(void *buffer, char relx, char rely)
{

    struct event_header *header = buffer;
    struct event_mousemove *mousemove = addpayload(buffer, sizeof (struct event_mousemove));

    mousemove->relx = relx;
    mousemove->rely = rely;

    return header->length;

}

unsigned int event_addconsoledata(void *buffer, char data)
{

    struct event_header *header = buffer;
    struct event_consoledata *consoledata = addpayload(buffer, sizeof (struct event_consoledata));

    consoledata->data = data;

    return header->length;

}

unsigned int event_addtimertick(void *buffer, unsigned int counter)
{

    struct event_header *header = buffer;
    struct event_timertick *timertick = addpayload(buffer, sizeof (struct event_timertick));

    timertick->counter = counter;

    return header->length;

}

unsigned int event_addvideomode(void *buffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_header *header = buffer;
    struct event_videomode *videomode = addpayload(buffer, sizeof (struct event_videomode));

    videomode->w = w;
    videomode->h = h;
    videomode->bpp = bpp;

    return header->length;

}

unsigned int event_addwmkeypress(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_wmkeypress *wmkeypress = addpayload(buffer, sizeof (struct event_wmkeypress));

    wmkeypress->scancode = scancode;

    return header->length;

}

unsigned int event_addwmkeyrelease(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_wmkeyrelease *wmkeyrelease = addpayload(buffer, sizeof (struct event_wmkeyrelease));

    wmkeyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addwmmousepress(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_wmmousepress *wmmousepress = addpayload(buffer, sizeof (struct event_wmmousepress));

    wmmousepress->button = button;

    return header->length;

}

unsigned int event_addwmmouserelease(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_wmmouserelease *wmmouserelease = addpayload(buffer, sizeof (struct event_wmmouserelease));

    wmmouserelease->button = button;

    return header->length;

}

unsigned int event_addwmmousemove(void *buffer, char relx, char rely)
{

    struct event_header *header = buffer;
    struct event_wmmousemove *wmmousemove = addpayload(buffer, sizeof (struct event_wmmousemove));

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    return header->length;

}

unsigned int event_addwmresize(void *buffer, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_header *header = buffer;
    struct event_wmresize *wmresize = addpayload(buffer, sizeof (struct event_wmresize));

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

    header->length += memory_write(buffer, FUDGE_BSIZE, data, count, header->length);

    return header->length;

}

