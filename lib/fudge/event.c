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

unsigned int event_addroute(struct event_header *header, unsigned int target)
{

    if (header->nroutes < 16)
        header->routes[header->nroutes++] = target;

    return header->nroutes;

}

unsigned int event_adddatapipe(struct event_header *header, unsigned int session)
{

    struct event_datapipe *datapipe = addpayload(header, sizeof (struct event_datapipe));

    datapipe->session = session;
    datapipe->count = 0;

    return header->length;

}

unsigned int event_adddatafile(struct event_header *header, unsigned int session, unsigned int descriptor)
{

    struct event_datafile *datafile = addpayload(header, sizeof (struct event_datafile));

    datafile->session = session;
    datafile->descriptor = descriptor;

    return header->length;

}

unsigned int event_adddatastop(struct event_header *header, unsigned int session)
{

    struct event_datastop *datastop = addpayload(header, sizeof (struct event_datastop));

    datastop->session = session;

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

unsigned int event_addwmconfigure(struct event_header *header, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_wmconfigure *wmconfigure = addpayload(header, sizeof (struct event_wmconfigure));

    wmconfigure->rendertarget = rendertarget;
    wmconfigure->x = x;
    wmconfigure->y = y;
    wmconfigure->w = w;
    wmconfigure->h = h;
    wmconfigure->padding = padding;
    wmconfigure->lineheight = lineheight;

    return header->length;

}

unsigned int event_appenddata(struct event_header *header, unsigned int count, void *buffer)
{

    struct event_datapipe *datapipe = event_getdata(header);
    unsigned int c = memory_write(header, FUDGE_BSIZE, buffer, count, header->length);

    datapipe->count += c;
    header->length += c;

    return header->length;

}

struct event_header *event_forward(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    unsigned int i;

    event_initheader(oheader, type, target);

    for (i = 0; i < iheader->nroutes; i++)
        event_addroute(oheader, iheader->routes[i]);

    return oheader;

}

struct event_header *event_forwarddatafile(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target, unsigned int descriptor)
{

    event_forward(oheader, iheader, EVENT_DATAFILE, target);
    event_adddatafile(oheader, session, descriptor);

    return oheader;

}

struct event_header *event_forwarddatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_forward(oheader, iheader, EVENT_DATASTOP, target);
    event_adddatastop(oheader, session);

    return oheader;

}

struct event_header *event_request(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    event_forward(oheader, iheader, type, target);
    event_addroute(oheader, iheader->target);

    return oheader;

}

struct event_header *event_requestdatafile(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target, unsigned int descriptor)
{

    event_request(oheader, iheader, EVENT_DATAFILE, target);
    event_adddatafile(oheader, session, descriptor);

    return oheader;

}

struct event_header *event_requestdatapipe(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_request(oheader, iheader, EVENT_DATAPIPE, target);
    event_adddatapipe(oheader, session);

    return oheader;

}

struct event_header *event_requestdatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_request(oheader, iheader, EVENT_DATASTOP, target);
    event_adddatastop(oheader, session);

    return oheader;

}

struct event_header *event_reply(struct event_header *oheader, struct event_header *iheader, unsigned int type)
{

    event_forward(oheader, iheader, type, iheader->source);

    if (oheader->nroutes)
    {

        oheader->nroutes--;
        oheader->target = oheader->routes[oheader->nroutes];

    }

    return oheader;

}

struct event_header *event_replydatapipe(struct event_header *oheader, struct event_header *iheader, unsigned int session)
{

    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, session);

    return oheader;

}

struct event_header *event_replydatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session)
{

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_adddatastop(oheader, session);

    return oheader;

}

