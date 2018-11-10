#include "define.h"
#include "memory.h"
#include "event.h"

static void *addpayload(union event_message *message, unsigned int length)
{

    char *address = message->buffer + message->header.length;

    message->header.length += length;
    message->header.plength += length;

    return address;

}

void *event_getdata(union event_message *message)
{

    return &message->header + 1;

}

unsigned int event_avail(union event_message *message)
{

    return FUDGE_BSIZE - message->header.length;

}

unsigned int event_route(union event_message *message, unsigned int target)
{

    if (message->header.nroutes < 16)
        message->header.routes[message->header.nroutes++] = target;

    return message->header.nroutes;

}

unsigned int event_addfile(union event_message *message, unsigned int descriptor)
{

    struct event_file *file = addpayload(message, sizeof (struct event_file));

    file->descriptor = descriptor;

    return message->header.length;

}

unsigned int event_addconsoledata(union event_message *message, char data)
{

    struct event_consoledata *consoledata = addpayload(message, sizeof (struct event_consoledata));

    consoledata->data = data;

    return message->header.length;

}

unsigned int event_addkeypress(union event_message *message, unsigned char scancode)
{

    struct event_keypress *keypress = addpayload(message, sizeof (struct event_keypress));

    keypress->scancode = scancode;

    return message->header.length;

}

unsigned int event_addkeyrelease(union event_message *message, unsigned char scancode)
{

    struct event_keyrelease *keyrelease = addpayload(message, sizeof (struct event_keyrelease));

    keyrelease->scancode = scancode;

    return message->header.length;

}

unsigned int event_addmousepress(union event_message *message, unsigned int button)
{

    struct event_mousepress *mousepress = addpayload(message, sizeof (struct event_mousepress));

    mousepress->button = button;

    return message->header.length;

}

unsigned int event_addmouserelease(union event_message *message, unsigned int button)
{

    struct event_mouserelease *mouserelease = addpayload(message, sizeof (struct event_mouserelease));

    mouserelease->button = button;

    return message->header.length;

}

unsigned int event_addmousemove(union event_message *message, char relx, char rely)
{

    struct event_mousemove *mousemove = addpayload(message, sizeof (struct event_mousemove));

    mousemove->relx = relx;
    mousemove->rely = rely;

    return message->header.length;

}

unsigned int event_addtimertick(union event_message *message, unsigned int counter)
{

    struct event_timertick *timertick = addpayload(message, sizeof (struct event_timertick));

    timertick->counter = counter;

    return message->header.length;

}

unsigned int event_addvideomode(union event_message *message, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_videomode *videomode = addpayload(message, sizeof (struct event_videomode));

    videomode->w = w;
    videomode->h = h;
    videomode->bpp = bpp;

    return message->header.length;

}

unsigned int event_addwmconfigure(union event_message *message, unsigned int rendertarget, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_wmconfigure *wmconfigure = addpayload(message, sizeof (struct event_wmconfigure));

    wmconfigure->rendertarget = rendertarget;
    wmconfigure->x = x;
    wmconfigure->y = y;
    wmconfigure->w = w;
    wmconfigure->h = h;
    wmconfigure->padding = padding;
    wmconfigure->lineheight = lineheight;

    return message->header.length;

}

unsigned int event_addwmkeypress(union event_message *message, unsigned char scancode)
{

    struct event_wmkeypress *wmkeypress = addpayload(message, sizeof (struct event_wmkeypress));

    wmkeypress->scancode = scancode;

    return message->header.length;

}

unsigned int event_addwmkeyrelease(union event_message *message, unsigned char scancode)
{

    struct event_wmkeyrelease *wmkeyrelease = addpayload(message, sizeof (struct event_wmkeyrelease));

    wmkeyrelease->scancode = scancode;

    return message->header.length;

}

unsigned int event_addwmmousepress(union event_message *message, unsigned int button)
{

    struct event_wmmousepress *wmmousepress = addpayload(message, sizeof (struct event_wmmousepress));

    wmmousepress->button = button;

    return message->header.length;

}

unsigned int event_addwmmouserelease(union event_message *message, unsigned int button)
{

    struct event_wmmouserelease *wmmouserelease = addpayload(message, sizeof (struct event_wmmouserelease));

    wmmouserelease->button = button;

    return message->header.length;

}

unsigned int event_addwmmousemove(union event_message *message, char relx, char rely)
{

    struct event_wmmousemove *wmmousemove = addpayload(message, sizeof (struct event_wmmousemove));

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    return message->header.length;

}

unsigned int event_append(union event_message *message, unsigned int count, void *buffer)
{

    unsigned int c = memory_write(message, FUDGE_BSIZE, buffer, count, message->header.length);

    message->header.length += c;
    message->header.plength += c;

    return message->header.length;

}

void event_create(union event_message *message, unsigned int type, unsigned int target, unsigned int session)
{

    message->header.type = type;
    message->header.source = 0;
    message->header.target = target;
    message->header.session = session;
    message->header.length = sizeof (struct event_header);
    message->header.plength = 0;
    message->header.nroutes = 0;

}

void event_forward(union event_message *omessage, union event_message *imessage, unsigned int type, unsigned int target)
{

    unsigned int i;

    event_create(omessage, type, target, imessage->header.session);

    for (i = 0; i < imessage->header.nroutes; i++)
        event_route(omessage, imessage->header.routes[i]);

}

void event_request(union event_message *omessage, union event_message *imessage, unsigned int type, unsigned int target, unsigned int session)
{

    unsigned int i;

    event_create(omessage, type, target, session);

    for (i = 0; i < imessage->header.nroutes; i++)
        event_route(omessage, imessage->header.routes[i]);

    event_route(omessage, imessage->header.target);

}

void event_reply(union event_message *omessage, union event_message *imessage, unsigned int type)
{

    unsigned int i;

    event_create(omessage, type, imessage->header.source, imessage->header.session);

    for (i = 0; i < imessage->header.nroutes; i++)
        event_route(omessage, imessage->header.routes[i]);

    if (omessage->header.nroutes)
        omessage->header.target = omessage->header.routes[--omessage->header.nroutes];

}

