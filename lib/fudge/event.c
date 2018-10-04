#include "define.h"
#include "memory.h"
#include "event.h"

void *event_getdata(struct event_header *header)
{

    return header + 1;

}

unsigned int event_avail(struct event_header *header)
{

    return FUDGE_BSIZE - header->length;

}

struct event_header *event_addheader(void *buffer, unsigned int type, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = 0;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->nroutes = 0;

    return header;

}

void *addpayload(struct event_header *header, unsigned int length)
{

    char *start = (char *)header;

    start += header->length;
    header->length += length;

    return start;

}

unsigned int event_addroute(void *buffer, unsigned int target)
{

    struct event_header *header = buffer;

    if (header->nroutes < 16)
        header->routes[header->nroutes++] = target;

    return header->nroutes;

}

struct event_header *event_request(void *buffer, struct event_header *header, unsigned int type, unsigned int target)
{

    struct event_header *request = event_addheader(buffer, type, target);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes; i++)
            event_addroute(buffer, header->routes[i]);

    }

    event_addroute(buffer, header->target);

    return request;

}

struct event_header *event_forward(void *buffer, struct event_header *header, unsigned int type, unsigned int target)
{

    struct event_header *forward = event_addheader(buffer, type, target);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes; i++)
            event_addroute(buffer, header->routes[i]);

    }

    return forward;

}

struct event_header *event_reply(void *buffer, struct event_header *header, unsigned int type)
{

    struct event_header *reply = event_addheader(buffer, type, header->source);

    if (header->nroutes)
    {

        unsigned int i;

        for (i = 0; i < header->nroutes - 1; i++)
            event_addroute(buffer, header->routes[i]);

        reply->target = header->routes[header->nroutes - 1];

    }

    return reply;

}

unsigned int event_adddata(struct event_header *header, unsigned int session)
{

    struct event_data *data = addpayload(header, sizeof (struct event_data));

    data->session = session;
    data->count = 0;

    return header->length;

}

unsigned int event_appenddata(struct event_header *header, unsigned int count, void *buffer)
{

    struct event_data *data = event_getdata(header);
    unsigned int c = memory_write(header, FUDGE_BSIZE, buffer, count, header->length);

    data->count += c;
    header->length += c;

    return header->length;

}

unsigned int event_addfile(struct event_header *header, unsigned int session, unsigned int descriptor)
{

    struct event_file *file = addpayload(header, sizeof (struct event_file));

    file->session = session;
    file->descriptor = descriptor;

    return header->length;

}

unsigned int event_addkeypress(struct event_header *header, unsigned char scancode)
{

    struct event_keypress *keypress = addpayload(header, sizeof (struct event_keypress));

    keypress->scancode = scancode;

    return header->length;

}

unsigned int event_addkeyrelease(struct event_header *header, unsigned char scancode)
{

    struct event_keyrelease *keyrelease = addpayload(header, sizeof (struct event_keyrelease));

    keyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addmousepress(struct event_header *header, unsigned int button)
{

    struct event_mousepress *mousepress = addpayload(header, sizeof (struct event_mousepress));

    mousepress->button = button;

    return header->length;

}

unsigned int event_addmouserelease(struct event_header *header, unsigned int button)
{

    struct event_mouserelease *mouserelease = addpayload(header, sizeof (struct event_mouserelease));

    mouserelease->button = button;

    return header->length;

}

unsigned int event_addmousemove(struct event_header *header, char relx, char rely)
{

    struct event_mousemove *mousemove = addpayload(header, sizeof (struct event_mousemove));

    mousemove->relx = relx;
    mousemove->rely = rely;

    return header->length;

}

unsigned int event_addconsoledata(struct event_header *header, char data)
{

    struct event_consoledata *consoledata = addpayload(header, sizeof (struct event_consoledata));

    consoledata->data = data;

    return header->length;

}

unsigned int event_addtimertick(struct event_header *header, unsigned int counter)
{

    struct event_timertick *timertick = addpayload(header, sizeof (struct event_timertick));

    timertick->counter = counter;

    return header->length;

}

unsigned int event_addvideomode(struct event_header *header, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videomode *videomode = addpayload(header, sizeof (struct event_videomode));

    videomode->w = w;
    videomode->h = h;
    videomode->bpp = bpp;

    return header->length;

}

unsigned int event_addwmkeypress(struct event_header *header, unsigned char scancode)
{

    struct event_wmkeypress *wmkeypress = addpayload(header, sizeof (struct event_wmkeypress));

    wmkeypress->scancode = scancode;

    return header->length;

}

unsigned int event_addwmkeyrelease(struct event_header *header, unsigned char scancode)
{

    struct event_wmkeyrelease *wmkeyrelease = addpayload(header, sizeof (struct event_wmkeyrelease));

    wmkeyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addwmmousepress(struct event_header *header, unsigned int button)
{

    struct event_wmmousepress *wmmousepress = addpayload(header, sizeof (struct event_wmmousepress));

    wmmousepress->button = button;

    return header->length;

}

unsigned int event_addwmmouserelease(struct event_header *header, unsigned int button)
{

    struct event_wmmouserelease *wmmouserelease = addpayload(header, sizeof (struct event_wmmouserelease));

    wmmouserelease->button = button;

    return header->length;

}

unsigned int event_addwmmousemove(struct event_header *header, char relx, char rely)
{

    struct event_wmmousemove *wmmousemove = addpayload(header, sizeof (struct event_wmmousemove));

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    return header->length;

}

unsigned int event_addwmresize(struct event_header *header, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_wmresize *wmresize = addpayload(header, sizeof (struct event_wmresize));

    wmresize->x = x;
    wmresize->y = y;
    wmresize->w = w;
    wmresize->h = h;
    wmresize->padding = padding;
    wmresize->lineheight = lineheight;

    return header->length;

}

unsigned int event_addwmflush(struct event_header *header, unsigned int count, void *buffer)
{

    header->length += memory_write(header, FUDGE_BSIZE, buffer, count, header->length);

    return header->length;

}

