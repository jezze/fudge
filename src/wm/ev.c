#include <abi.h>
#include <fudge.h>
#include "ev.h"

static void onkeypress(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_keypress keypress;

    file_readall(descriptor, &keypress, sizeof (struct event_keypress));

    if (handlers->keypress)
        handlers->keypress(header, &keypress);

}

static void onkeyrelease(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(descriptor, &keyrelease, sizeof (struct event_keyrelease));

    if (handlers->keyrelease)
        handlers->keyrelease(header, &keyrelease);

}

static void onmousemove(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mousemove mousemove;

    file_readall(descriptor, &mousemove, sizeof (struct event_mousemove));

    if (handlers->mousemove)
        handlers->mousemove(header, &mousemove);

}

static void onmousepress(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mousepress mousepress;

    file_readall(descriptor, &mousepress, sizeof (struct event_mousepress));

    if (handlers->mousepress)
        handlers->mousepress(header, &mousepress);

}

static void onmouserelease(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mouserelease mouserelease;

    file_readall(descriptor, &mouserelease, sizeof (struct event_mouserelease));

    if (handlers->mouserelease)
        handlers->mouserelease(header, &mouserelease);

}

static void ontick(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_tick tick;

    file_readall(descriptor, &tick, sizeof (struct event_tick));

    if (handlers->tick)
        handlers->tick(header, &tick);

}

static void onvideomode(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_videomode videomode;

    file_readall(descriptor, &videomode, sizeof (struct event_videomode));

    if (handlers->videomode)
        handlers->videomode(header, &videomode);

}

static void onwmmap(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmmap)
        handlers->wmmap(header);

}

static void onwmunmap(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmunmap)
        handlers->wmunmap(header);

}

static void onwmresize(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmresize wmresize;

    file_readall(descriptor, &wmresize, sizeof (struct event_wmresize));

    if (handlers->wmresize)
        handlers->wmresize(header, &wmresize);

}

static void onwmshow(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmshow)
        handlers->wmshow(header);

}

static void onwmhide(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmhide)
        handlers->wmhide(header);

}

static void (*funcs[EVENTS])(struct ev_handlers *handlers, unsigned int descriptor, struct event_header *header) = {
    0,
    onkeypress,
    onkeyrelease,
    onmousemove,
    onmousepress,
    onmouserelease,
    ontick,
    onvideomode,
    onwmmap,
    onwmunmap,
    onwmresize,
    onwmshow,
    onwmhide
};

unsigned int ev_read(struct ev_handlers *handlers, unsigned int descriptor)
{

    struct event_header header;
    unsigned int count = file_readall(descriptor, &header, sizeof (struct event_header));

    if (!count)
        return 0;

    if (funcs[header.type])
        funcs[header.type](handlers, descriptor, &header);

    return count;

}

static void send(unsigned int descriptor, unsigned int destination, unsigned int type, unsigned int datacount, void *databuffer)
{

    unsigned char buffer[512];
    struct event_header header;

    header.destination = destination;
    header.type = type;

    memory_write(buffer, 512, &header, sizeof (struct event_header), 0);
    memory_write(buffer, 512, databuffer, datacount, sizeof (struct event_header));
    file_writeall(descriptor, buffer, sizeof (struct event_header) + datacount);

}

void ev_sendkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keypress keypress;

    keypress.scancode = scancode;

    send(descriptor, destination, EVENT_KEYPRESS, sizeof (struct event_keypress), &keypress);

}

void ev_sendkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct event_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    send(descriptor, destination, EVENT_KEYRELEASE, sizeof (struct event_keyrelease), &keyrelease);

}

void ev_sendmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely)
{

    struct event_mousemove mousemove;

    mousemove.relx = relx;
    mousemove.rely = rely;

    send(descriptor, destination, EVENT_MOUSEMOVE, sizeof (struct event_mousemove), &mousemove);

}

void ev_sendmousepress(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct event_mousepress mousepress;

    mousepress.button = button;

    send(descriptor, destination, EVENT_MOUSEPRESS, sizeof (struct event_mousepress), &mousepress);

}

void ev_sendmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct event_mouserelease mouserelease;

    mouserelease.button = button;

    send(descriptor, destination, EVENT_MOUSERELEASE, sizeof (struct event_mouserelease), &mouserelease);

}

void ev_sendwmmap(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMMAP, 0, 0);

}

void ev_sendwmunmap(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMUNMAP, 0, 0);

}

void ev_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct event_wmresize wmresize;

    wmresize.x = x;
    wmresize.y = y;
    wmresize.w = w;
    wmresize.h = h;

    send(descriptor, destination, EVENT_WMRESIZE, sizeof (struct event_wmresize), &wmresize);

}

void ev_sendwmshow(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMSHOW, 0, 0);

}

void ev_sendwmhide(unsigned int descriptor, unsigned int destination)
{

    send(descriptor, destination, EVENT_WMHIDE, 0, 0);

}

