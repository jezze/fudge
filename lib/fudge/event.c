#include "define.h"
#include "memory.h"
#include "event.h"

void *event_getforward(void *buffer)
{

    struct event_header *header = buffer;

    return header + 1;

}

void *event_getdata(void *buffer)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    return forward + header->forward;

}

unsigned int event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = source;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->forward = 0;

    return header->length;

}

unsigned int event_addforward(void *buffer, unsigned int target)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    forward[header->forward].target = target;
    header->forward++;
    header->length += sizeof (struct event_forward);

    return header->length;

}

unsigned int event_addrequest(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, id);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward; i++)
            event_addforward(buffer, forward[i].target);

    }

    event_addforward(buffer, header->target);

    return reply->length;

}

unsigned int event_addpipe(void *buffer, struct event_header *header, unsigned int type, unsigned int id)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, id);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward; i++)
            event_addforward(buffer, forward[i].target);

    }

    return reply->length;

}

unsigned int event_addresponse(void *buffer, struct event_header *header, unsigned int type)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, header->source);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        for (i = 0; i < header->forward - 1; i++)
            event_addforward(buffer, forward[i].target);

        reply->target = forward[header->forward - 1].target;

    }

    return reply->length;

}

unsigned int event_adddata(void *buffer, unsigned int stream, unsigned int count, void *data)
{

    struct event_header *header = buffer;
    struct event_data *d = event_getdata(buffer);

    header->length += sizeof (struct event_data);
    d->stream = stream;
    d->count = memory_write(buffer, FUDGE_BSIZE, data, count, header->length);
    header->length += d->count;

    return header->length;

}

unsigned int event_addfile(void *buffer, unsigned int stream, unsigned int descriptor)
{

    struct event_header *header = buffer;
    struct event_file *file = event_getdata(buffer);

    header->length += sizeof (struct event_file);
    file->stream = stream;
    file->descriptor = descriptor;

    return header->length;

}

unsigned int event_addkeypress(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_keypress *keypress = event_getdata(buffer);

    header->length += sizeof (struct event_keypress);
    keypress->scancode = scancode;

    return header->length;

}

unsigned int event_addkeyrelease(void *buffer, unsigned char scancode)
{

    struct event_header *header = buffer;
    struct event_keyrelease *keyrelease = event_getdata(buffer);

    header->length += sizeof (struct event_keyrelease);
    keyrelease->scancode = scancode;

    return header->length;

}

unsigned int event_addmousepress(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_mousepress *mousepress = event_getdata(buffer);

    header->length += sizeof (struct event_mousepress);
    mousepress->button = button;

    return header->length;

}

unsigned int event_addmouserelease(void *buffer, unsigned int button)
{

    struct event_header *header = buffer;
    struct event_mouserelease *mouserelease = event_getdata(buffer);

    header->length += sizeof (struct event_mouserelease);
    mouserelease->button = button;

    return header->length;

}

unsigned int event_addmousemove(void *buffer, char relx, char rely)
{

    struct event_header *header = buffer;
    struct event_mousemove *mousemove = event_getdata(buffer);

    header->length += sizeof (struct event_mousemove);
    mousemove->relx = relx;
    mousemove->rely = rely;

    return header->length;

}

unsigned int event_addconsoledata(void *buffer, char data)
{

    struct event_header *header = buffer;
    struct event_consoledata *consoledata = event_getdata(buffer);

    header->length += sizeof (struct event_consoledata);
    consoledata->data = data;

    return header->length;

}

unsigned int event_addtimertick(void *buffer, unsigned int counter)
{

    struct event_header *header = buffer;
    struct event_timertick *timertick = event_getdata(buffer);

    header->length += sizeof (struct event_timertick);
    timertick->counter = counter;

    return header->length;

}

unsigned int event_addvideomode(void *buffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct event_header *header = buffer;
    struct event_videomode *videomode = event_getdata(buffer);

    header->length += sizeof (struct event_videomode);
    videomode->w = w;
    videomode->h = h;
    videomode->bpp = bpp;

    return header->length;

}

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

