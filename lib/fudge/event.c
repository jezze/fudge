#include "define.h"
#include "memory.h"
#include "event.h"

static void *addpayload(struct event_header *header, unsigned int length)
{

    char *start = (char *)header;

    start += header->length;
    header->length += length;

    return start;

}

void *event_getdata(struct event_header *header)
{

    return header + 1;

}

unsigned int event_avail(struct event_header *header)
{

    return FUDGE_BSIZE - header->length;

}

void event_initheader(struct event_header *header, unsigned int type, unsigned int target)
{

    header->type = type;
    header->source = 0;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->nroutes = 0;

}

struct event_header *event_request(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    event_initheader(oheader, type, target);

    if (iheader->nroutes)
    {

        unsigned int i;

        for (i = 0; i < iheader->nroutes; i++)
            event_addroute(oheader, iheader->routes[i]);

    }

    event_addroute(oheader, iheader->target);

    return oheader;

}

struct event_header *event_forward(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    event_initheader(oheader, type, target);

    if (iheader->nroutes)
    {

        unsigned int i;

        for (i = 0; i < iheader->nroutes; i++)
            event_addroute(oheader, iheader->routes[i]);

    }

    return oheader;

}

struct event_header *event_reply(struct event_header *oheader, struct event_header *iheader, unsigned int type)
{

    event_initheader(oheader, type, iheader->source);

    if (iheader->nroutes)
    {

        unsigned int i;

        for (i = 0; i < iheader->nroutes - 1; i++)
            event_addroute(oheader, iheader->routes[i]);

        oheader->target = iheader->routes[iheader->nroutes - 1];

    }

    return oheader;

}

unsigned int event_addroute(struct event_header *header, unsigned int target)
{

    if (header->nroutes < 16)
        header->routes[header->nroutes++] = target;

    return header->nroutes;

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

