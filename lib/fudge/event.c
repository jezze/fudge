#include "define.h"
#include "memory.h"
#include "event.h"

static void *addpayload(struct event_header *header, unsigned int length)
{

    header->length += length;

    return header + 1;

}

unsigned int event_avail(struct event_header *header)
{

    return FUDGE_BSIZE - header->length;

}

unsigned int event_addroute(struct event_header *header, unsigned int target, unsigned int session)
{

    if (header->nroutes < 16)
    {

        header->routes[header->nroutes].target = target;
        header->routes[header->nroutes].session = session;
        header->nroutes++;

    }

    return header->nroutes;

}

unsigned int event_addfile(union event_message *message, unsigned int descriptor)
{

    struct event_file *file = addpayload(&message->header, sizeof (struct event_file));

    file->descriptor = descriptor;

    return message->header.length;

}

unsigned int event_addblockrequest(union event_message *message, unsigned int offset, unsigned int count)
{

    struct event_blockrequest *blockrequest = addpayload(&message->header, sizeof (struct event_blockrequest));

    blockrequest->offset = offset;
    blockrequest->count = count;

    return message->header.length;

}

unsigned int event_addwmconfigure(union event_message *message, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct event_wmconfigure *wmconfigure = addpayload(&message->header, sizeof (struct event_wmconfigure));

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

    struct event_wmkeypress *wmkeypress = addpayload(&message->header, sizeof (struct event_wmkeypress));

    wmkeypress->scancode = scancode;

    return message->header.length;

}

unsigned int event_addwmkeyrelease(union event_message *message, unsigned char scancode)
{

    struct event_wmkeyrelease *wmkeyrelease = addpayload(&message->header, sizeof (struct event_wmkeyrelease));

    wmkeyrelease->scancode = scancode;

    return message->header.length;

}

unsigned int event_addwmmousepress(union event_message *message, unsigned int button)
{

    struct event_wmmousepress *wmmousepress = addpayload(&message->header, sizeof (struct event_wmmousepress));

    wmmousepress->button = button;

    return message->header.length;

}

unsigned int event_addwmmouserelease(union event_message *message, unsigned int button)
{

    struct event_wmmouserelease *wmmouserelease = addpayload(&message->header, sizeof (struct event_wmmouserelease));

    wmmouserelease->button = button;

    return message->header.length;

}

unsigned int event_addwmmousemove(union event_message *message, char relx, char rely)
{

    struct event_wmmousemove *wmmousemove = addpayload(&message->header, sizeof (struct event_wmmousemove));

    wmmousemove->relx = relx;
    wmmousemove->rely = rely;

    return message->header.length;

}

unsigned int event_append(struct event_header *header, unsigned int count, void *buffer)
{

    return header->length += memory_write(header, FUDGE_BSIZE, buffer, count, header->length);

}

void event_reset(struct event_header *header)
{

    header->length = sizeof (struct event_header);

}

void event_create(struct event_header *header, unsigned int type)
{

    header->type = type;
    header->source = 0;
    header->target = 0;
    header->session = 0;
    header->length = sizeof (struct event_header);
    header->nroutes = 0;

}

void event_create2(struct event_header *header, unsigned int type, unsigned int length)
{

    event_create(header, type);
    addpayload(header, length);

}

