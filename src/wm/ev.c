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

