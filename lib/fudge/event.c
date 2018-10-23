#include "define.h"
#include "memory.h"
#include "event.h"

static void initheader(struct event_header *header, unsigned int type, unsigned int target)
{

    header->type = type;
    header->source = 0;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->nroutes = 0;

}

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

unsigned int event_route(struct event_header *header, unsigned int target)
{

    if (header->nroutes < 16)
        header->routes[header->nroutes++] = target;

    return header->nroutes;

}

static unsigned int addconsoledata(struct event_header *header, char data)
{

    struct event_consoledata *consoledata = addpayload(header, sizeof (struct event_consoledata));

    consoledata->data = data;

    return header->length;

}

static unsigned int adddatafile(struct event_header *header, unsigned int session, unsigned int descriptor)
{

    struct event_datafile *datafile = addpayload(header, sizeof (struct event_datafile));

    datafile->session = session;
    datafile->descriptor = descriptor;

    return header->length;

}

static unsigned int adddatapipe(struct event_header *header, unsigned int session)
{

    struct event_datapipe *datapipe = addpayload(header, sizeof (struct event_datapipe));

    datapipe->session = session;
    datapipe->count = 0;

    return header->length;

}

static unsigned int adddatastop(struct event_header *header, unsigned int session)
{

    struct event_datastop *datastop = addpayload(header, sizeof (struct event_datastop));

    datastop->session = session;

    return header->length;

}

static unsigned int addkeypress(struct event_header *header, unsigned char scancode)
{

    struct event_keypress *keypress = addpayload(header, sizeof (struct event_keypress));

    keypress->scancode = scancode;

    return header->length;

}

static unsigned int addkeyrelease(struct event_header *header, unsigned char scancode)
{

    struct event_keyrelease *keyrelease = addpayload(header, sizeof (struct event_keyrelease));

    keyrelease->scancode = scancode;

    return header->length;

}

static unsigned int addmousepress(struct event_header *header, unsigned int button)
{

    struct event_mousepress *mousepress = addpayload(header, sizeof (struct event_mousepress));

    mousepress->button = button;

    return header->length;

}

static unsigned int addmouserelease(struct event_header *header, unsigned int button)
{

    struct event_mouserelease *mouserelease = addpayload(header, sizeof (struct event_mouserelease));

    mouserelease->button = button;

    return header->length;

}

static unsigned int addmousemove(struct event_header *header, char relx, char rely)
{

    struct event_mousemove *mousemove = addpayload(header, sizeof (struct event_mousemove));

    mousemove->relx = relx;
    mousemove->rely = rely;

    return header->length;

}

static unsigned int addtimertick(struct event_header *header, unsigned int counter)
{

    struct event_timertick *timertick = addpayload(header, sizeof (struct event_timertick));

    timertick->counter = counter;

    return header->length;

}

static unsigned int addvideomode(struct event_header *header, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videomode *videomode = addpayload(header, sizeof (struct event_videomode));

    videomode->w = w;
    videomode->h = h;
    videomode->bpp = bpp;

    return header->length;

}

static unsigned int addwmconfigure(struct event_header *header, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
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

static unsigned int addwmkeypress(struct event_header *header, unsigned char scancode)
{

    struct event_wmkeypress *wmkeypress = addpayload(header, sizeof (struct event_wmkeypress));

    wmkeypress->scancode = scancode;

    return header->length;

}

static unsigned int addwmkeyrelease(struct event_header *header, unsigned char scancode)
{

    struct event_wmkeyrelease *wmkeyrelease = addpayload(header, sizeof (struct event_wmkeyrelease));

    wmkeyrelease->scancode = scancode;

    return header->length;

}

static unsigned int addwmmousepress(struct event_header *header, unsigned int button)
{

    struct event_wmmousepress *wmmousepress = addpayload(header, sizeof (struct event_wmmousepress));

    wmmousepress->button = button;

    return header->length;

}

static unsigned int addwmmouserelease(struct event_header *header, unsigned int button)
{

    struct event_wmmouserelease *wmmouserelease = addpayload(header, sizeof (struct event_wmmouserelease));

    wmmouserelease->button = button;

    return header->length;

}

static unsigned int addwmmousemove(struct event_header *header, char relx, char rely)
{

    struct event_wmmousemove *wmmousemove = addpayload(header, sizeof (struct event_wmmousemove));

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

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

struct event_header *event_create(struct event_header *oheader, unsigned int type, unsigned int target)
{

    initheader(oheader, type, target);

    return oheader;

}

struct event_header *event_createconsoledata(struct event_header *oheader, unsigned int target, char data)
{

    event_create(oheader, EVENT_CONSOLEDATA, target);
    addconsoledata(oheader, data);

    return oheader;

}

struct event_header *event_createdatafile(struct event_header *oheader, unsigned int session, unsigned int target, unsigned int descriptor)
{

    event_create(oheader, EVENT_DATAFILE, target);
    adddatafile(oheader, session, descriptor);

    return oheader;

}

struct event_header *event_createdatapipe(struct event_header *oheader, unsigned int session, unsigned int target)
{

    event_create(oheader, EVENT_DATAPIPE, target);
    adddatapipe(oheader, session);

    return oheader;

}

struct event_header *event_createdatastop(struct event_header *oheader, unsigned int session, unsigned int target)
{

    event_create(oheader, EVENT_DATASTOP, target);
    adddatastop(oheader, session);

    return oheader;

}

struct event_header *event_createinit(struct event_header *oheader, unsigned int target)
{

    event_create(oheader, EVENT_INIT, target);

    return oheader;

}

struct event_header *event_createkeypress(struct event_header *oheader, unsigned int target, unsigned char scancode)
{

    event_create(oheader, EVENT_KEYPRESS, target);
    addkeypress(oheader, scancode);

    return oheader;

}

struct event_header *event_createkeyrelease(struct event_header *oheader, unsigned int target, unsigned char scancode)
{

    event_create(oheader, EVENT_KEYRELEASE, target);
    addkeyrelease(oheader, scancode);

    return oheader;

}

struct event_header *event_createmousepress(struct event_header *oheader, unsigned int target, unsigned int button)
{

    event_create(oheader, EVENT_MOUSEPRESS, target);
    addmousepress(oheader, button);

    return oheader;

}

struct event_header *event_createmouserelease(struct event_header *oheader, unsigned int target, unsigned int button)
{

    event_create(oheader, EVENT_MOUSERELEASE, target);
    addmouserelease(oheader, button);

    return oheader;

}

struct event_header *event_createmousemove(struct event_header *oheader, unsigned int target, char relx, char rely)
{

    event_create(oheader, EVENT_MOUSEMOVE, target);
    addmousemove(oheader, relx, rely);

    return oheader;

}

struct event_header *event_createtimertick(struct event_header *oheader, unsigned int target, unsigned int counter)
{

    event_create(oheader, EVENT_TIMERTICK, target);
    addtimertick(oheader, counter);

    return oheader;

}

struct event_header *event_createvideomode(struct event_header *oheader, unsigned int target, unsigned int w, unsigned int h, unsigned int bpp)
{

    event_create(oheader, EVENT_VIDEOMODE, target);
    addvideomode(oheader, w, h, bpp);

    return oheader;

}

struct event_header *event_forward(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    unsigned int i;

    event_create(oheader, type, target);

    for (i = 0; i < iheader->nroutes; i++)
        event_route(oheader, iheader->routes[i]);

    return oheader;

}

struct event_header *event_forwarddatafile(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target, unsigned int descriptor)
{

    event_forward(oheader, iheader, EVENT_DATAFILE, target);
    adddatafile(oheader, session, descriptor);

    return oheader;

}

struct event_header *event_forwarddatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_forward(oheader, iheader, EVENT_DATASTOP, target);
    adddatastop(oheader, session);

    return oheader;

}

struct event_header *event_forwardinit(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_forward(oheader, iheader, EVENT_INIT, target);

    return oheader;

}

struct event_header *event_request(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    event_forward(oheader, iheader, type, target);
    event_route(oheader, iheader->target);

    return oheader;

}

struct event_header *event_requestdatafile(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target, unsigned int descriptor)
{

    event_request(oheader, iheader, EVENT_DATAFILE, target);
    adddatafile(oheader, session, descriptor);

    return oheader;

}

struct event_header *event_requestdatapipe(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_request(oheader, iheader, EVENT_DATAPIPE, target);
    adddatapipe(oheader, session);

    return oheader;

}

struct event_header *event_requestdatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session, unsigned int target)
{

    event_request(oheader, iheader, EVENT_DATASTOP, target);
    adddatastop(oheader, session);

    return oheader;

}

struct event_header *event_requestinit(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_INIT, target);

    return oheader;

}

struct event_header *event_requestkill(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_KILL, target);

    return oheader;

}

struct event_header *event_requestwmshow(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_WMSHOW, target);

    return oheader;

}

struct event_header *event_requestwmhide(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_WMHIDE, target);

    return oheader;

}

struct event_header *event_requestwmmap(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_WMMAP, target);

    return oheader;

}

struct event_header *event_requestwmunmap(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_request(oheader, iheader, EVENT_WMUNMAP, target);

    return oheader;

}

struct event_header *event_requestwmkeypress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned char scancode)
{

    event_request(oheader, iheader, EVENT_WMKEYPRESS, target);
    addwmkeypress(oheader, scancode);

    return oheader;

}

struct event_header *event_requestwmkeyrelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned char scancode)
{

    event_request(oheader, iheader, EVENT_WMKEYRELEASE, target);
    addwmkeyrelease(oheader, scancode);

    return oheader;

}

struct event_header *event_requestwmmousepress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int button)
{

    event_request(oheader, iheader, EVENT_WMMOUSEPRESS, target);
    addwmmousepress(oheader, button);

    return oheader;

}

struct event_header *event_requestwmmouserelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int button)
{

    event_request(oheader, iheader, EVENT_WMMOUSERELEASE, target);
    addwmmouserelease(oheader, button);

    return oheader;

}

struct event_header *event_requestwmmousemove(struct event_header *oheader, struct event_header *iheader, unsigned int target, char relx, char rely)
{

    event_request(oheader, iheader, EVENT_WMMOUSEMOVE, target);
    addwmmousemove(oheader, relx, rely);

    return oheader;

}

struct event_header *event_requestwmconfigure(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    event_request(oheader, iheader, EVENT_WMCONFIGURE, target);
    addwmconfigure(oheader, rendertarget, x, y, w, h, padding, lineheight);

    return oheader;

}

struct event_header *event_reply(struct event_header *oheader, struct event_header *iheader, unsigned int type)
{

    event_forward(oheader, iheader, type, iheader->source);

    if (oheader->nroutes)
        oheader->target = oheader->routes[--oheader->nroutes];

    return oheader;

}

struct event_header *event_replydatapipe(struct event_header *oheader, struct event_header *iheader, unsigned int session)
{

    event_reply(oheader, iheader, EVENT_DATAPIPE);
    adddatapipe(oheader, session);

    return oheader;

}

struct event_header *event_replydatastop(struct event_header *oheader, struct event_header *iheader, unsigned int session)
{

    event_reply(oheader, iheader, EVENT_DATASTOP);
    adddatastop(oheader, session);

    return oheader;

}

