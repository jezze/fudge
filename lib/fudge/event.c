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

unsigned int event_route(struct event_header *header, unsigned int target)
{

    if (header->nroutes < 16)
        header->routes[header->nroutes++] = target;

    return header->nroutes;

}

static unsigned int addfile(struct event_header *header, unsigned int descriptor)
{

    struct event_file *file = addpayload(header, sizeof (struct event_file));

    file->descriptor = descriptor;

    return header->length;

}

static unsigned int adddata(struct event_header *header)
{

    struct event_data *data = addpayload(header, sizeof (struct event_data));

    data->count = 0;

    return header->length;

}

static unsigned int addconsoledata(struct event_header *header, char data)
{

    struct event_consoledata *consoledata = addpayload(header, sizeof (struct event_consoledata));

    consoledata->data = data;

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

    struct event_data *data = event_getdata(header);
    unsigned int c = memory_write(header, FUDGE_BSIZE, buffer, count, header->length);

    data->count += c;
    header->length += c;

    return header->length;

}

struct event_header *event_create(struct event_header *oheader, unsigned int type, unsigned int target, unsigned int session)
{

    oheader->type = type;
    oheader->source = 0;
    oheader->target = target;
    oheader->session = session;
    oheader->length = sizeof (struct event_header);
    oheader->nroutes = 0;

    return oheader;

}

struct event_header *event_createfile(struct event_header *oheader, unsigned int target, unsigned int session, unsigned int descriptor)
{

    event_create(oheader, EVENT_FILE, target, session);
    addfile(oheader, descriptor);

    return oheader;

}

struct event_header *event_createdata(struct event_header *oheader, unsigned int target, unsigned int session)
{

    event_create(oheader, EVENT_DATA, target, session);
    adddata(oheader);

    return oheader;

}

struct event_header *event_createstop(struct event_header *oheader, unsigned int target, unsigned int session)
{

    event_create(oheader, EVENT_STOP, target, session);

    return oheader;

}

struct event_header *event_createinit(struct event_header *oheader, unsigned int target, unsigned int session)
{

    event_create(oheader, EVENT_INIT, target, session);

    return oheader;

}

struct event_header *event_createconsoledata(struct event_header *oheader, char data)
{

    event_create(oheader, EVENT_CONSOLEDATA, EVENT_BROADCAST, 0);
    addconsoledata(oheader, data);

    return oheader;

}

struct event_header *event_createkeypress(struct event_header *oheader, unsigned char scancode)
{

    event_create(oheader, EVENT_KEYPRESS, EVENT_BROADCAST, 0);
    addkeypress(oheader, scancode);

    return oheader;

}

struct event_header *event_createkeyrelease(struct event_header *oheader, unsigned char scancode)
{

    event_create(oheader, EVENT_KEYRELEASE, EVENT_BROADCAST, 0);
    addkeyrelease(oheader, scancode);

    return oheader;

}

struct event_header *event_createmousepress(struct event_header *oheader, unsigned int button)
{

    event_create(oheader, EVENT_MOUSEPRESS, EVENT_BROADCAST, 0);
    addmousepress(oheader, button);

    return oheader;

}

struct event_header *event_createmouserelease(struct event_header *oheader, unsigned int button)
{

    event_create(oheader, EVENT_MOUSERELEASE, EVENT_BROADCAST, 0);
    addmouserelease(oheader, button);

    return oheader;

}

struct event_header *event_createmousemove(struct event_header *oheader, char relx, char rely)
{

    event_create(oheader, EVENT_MOUSEMOVE, EVENT_BROADCAST, 0);
    addmousemove(oheader, relx, rely);

    return oheader;

}

struct event_header *event_createtimertick(struct event_header *oheader, unsigned int counter)
{

    event_create(oheader, EVENT_TIMERTICK, EVENT_BROADCAST, 0);
    addtimertick(oheader, counter);

    return oheader;

}

struct event_header *event_createvideomode(struct event_header *oheader, unsigned int w, unsigned int h, unsigned int bpp)
{

    event_create(oheader, EVENT_VIDEOMODE, EVENT_BROADCAST, 0);
    addvideomode(oheader, w, h, bpp);

    return oheader;

}

struct event_header *event_forward(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target)
{

    unsigned int i;

    event_create(oheader, type, target, iheader->session);

    for (i = 0; i < iheader->nroutes; i++)
        event_route(oheader, iheader->routes[i]);

    return oheader;

}

struct event_header *event_forwardfile(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int descriptor)
{

    event_forward(oheader, iheader, EVENT_FILE, target);
    addfile(oheader, descriptor);

    return oheader;

}

struct event_header *event_forwardstop(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_forward(oheader, iheader, EVENT_STOP, target);

    return oheader;

}

struct event_header *event_forwardinit(struct event_header *oheader, struct event_header *iheader, unsigned int target)
{

    event_forward(oheader, iheader, EVENT_INIT, target);

    return oheader;

}

struct event_header *event_request(struct event_header *oheader, struct event_header *iheader, unsigned int type, unsigned int target, unsigned int session)
{

    unsigned int i;

    event_create(oheader, type, target, session);

    for (i = 0; i < iheader->nroutes; i++)
        event_route(oheader, iheader->routes[i]);

    event_route(oheader, iheader->target);

    return oheader;

}

struct event_header *event_requestfile(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int descriptor)
{

    event_request(oheader, iheader, EVENT_FILE, target, session);
    addfile(oheader, descriptor);

    return oheader;

}

struct event_header *event_requestdata(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_DATA, target, session);
    adddata(oheader);

    return oheader;

}

struct event_header *event_requeststop(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_STOP, target, session);

    return oheader;

}

struct event_header *event_requestinit(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_INIT, target, session);

    return oheader;

}

struct event_header *event_requestkill(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_KILL, target, session);

    return oheader;

}

struct event_header *event_requestwmshow(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_WMSHOW, target, session);

    return oheader;

}

struct event_header *event_requestwmhide(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_WMHIDE, target, session);

    return oheader;

}

struct event_header *event_requestwmmap(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_WMMAP, target, session);

    return oheader;

}

struct event_header *event_requestwmunmap(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session)
{

    event_request(oheader, iheader, EVENT_WMUNMAP, target, session);

    return oheader;

}

struct event_header *event_requestwmkeypress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned char scancode)
{

    event_request(oheader, iheader, EVENT_WMKEYPRESS, target, session);
    addwmkeypress(oheader, scancode);

    return oheader;

}

struct event_header *event_requestwmkeyrelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned char scancode)
{

    event_request(oheader, iheader, EVENT_WMKEYRELEASE, target, session);
    addwmkeyrelease(oheader, scancode);

    return oheader;

}

struct event_header *event_requestwmmousepress(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int button)
{

    event_request(oheader, iheader, EVENT_WMMOUSEPRESS, target, session);
    addwmmousepress(oheader, button);

    return oheader;

}

struct event_header *event_requestwmmouserelease(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int button)
{

    event_request(oheader, iheader, EVENT_WMMOUSERELEASE, target, session);
    addwmmouserelease(oheader, button);

    return oheader;

}

struct event_header *event_requestwmmousemove(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, char relx, char rely)
{

    event_request(oheader, iheader, EVENT_WMMOUSEMOVE, target, session);
    addwmmousemove(oheader, relx, rely);

    return oheader;

}

struct event_header *event_requestwmconfigure(struct event_header *oheader, struct event_header *iheader, unsigned int target, unsigned int session, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    event_request(oheader, iheader, EVENT_WMCONFIGURE, target, session);
    addwmconfigure(oheader, rendertarget, x, y, w, h, padding, lineheight);

    return oheader;

}

struct event_header *event_reply(struct event_header *oheader, struct event_header *iheader, unsigned int type)
{

    unsigned int i;

    event_create(oheader, type, iheader->source, iheader->session);

    for (i = 0; i < iheader->nroutes; i++)
        event_route(oheader, iheader->routes[i]);

    if (oheader->nroutes)
        oheader->target = oheader->routes[--oheader->nroutes];

    return oheader;

}

struct event_header *event_replydata(struct event_header *oheader, struct event_header *iheader)
{

    event_reply(oheader, iheader, EVENT_DATA);
    adddata(oheader);

    return oheader;

}

struct event_header *event_replystop(struct event_header *oheader, struct event_header *iheader)
{

    event_reply(oheader, iheader, EVENT_STOP);

    return oheader;

}

