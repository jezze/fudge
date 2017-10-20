#include <abi.h>
#include <fudge.h>
#include "event.h"

static void onkeypress(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_keypress keypress;

    file_readall(descriptor, &keypress, sizeof (struct event_keypress));

    if (handlers->keypress)
        handlers->keypress(header, &keypress);

}

static void onkeyrelease(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(descriptor, &keyrelease, sizeof (struct event_keyrelease));

    if (handlers->keyrelease)
        handlers->keyrelease(header, &keyrelease);

}

static void onmousemove(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mousemove mousemove;

    file_readall(descriptor, &mousemove, sizeof (struct event_mousemove));

    if (handlers->mousemove)
        handlers->mousemove(header, &mousemove);

}

static void onmousepress(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mousepress mousepress;

    file_readall(descriptor, &mousepress, sizeof (struct event_mousepress));

    if (handlers->mousepress)
        handlers->mousepress(header, &mousepress);

}

static void onmouserelease(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_mouserelease mouserelease;

    file_readall(descriptor, &mouserelease, sizeof (struct event_mouserelease));

    if (handlers->mouserelease)
        handlers->mouserelease(header, &mouserelease);

}

static void ontimertick(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_timertick timertick;

    file_readall(descriptor, &timertick, sizeof (struct event_timertick));

    if (handlers->timertick)
        handlers->timertick(header, &timertick);

}

static void onvideomode(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_videomode videomode;

    file_readall(descriptor, &videomode, sizeof (struct event_videomode));

    if (handlers->videomode)
        handlers->videomode(header, &videomode);

}

static void onwmmap(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmmap)
        handlers->wmmap(header);

}

static void onwmunmap(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmunmap)
        handlers->wmunmap(header);

}

static void onwminit(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wminit)
        handlers->wminit(header);

}

static void onwmexit(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmexit)
        handlers->wmexit(header);

}

static void onwmresize(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmresize wmresize;

    file_readall(descriptor, &wmresize, sizeof (struct event_wmresize));

    if (handlers->wmresize)
        handlers->wmresize(header, &wmresize);

}

static void onwmshow(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmshow)
        handlers->wmshow(header);

}

static void onwmhide(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmhide)
        handlers->wmhide(header);

}

static void onwmflush(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    if (handlers->wmflush)
        handlers->wmflush(header);

}

static void onwmkeypress(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmkeypress wmkeypress;

    file_readall(descriptor, &wmkeypress, sizeof (struct event_wmkeypress));

    if (handlers->wmkeypress)
        handlers->wmkeypress(header, &wmkeypress);

}

static void onwmkeyrelease(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmkeyrelease wmkeyrelease;

    file_readall(descriptor, &wmkeyrelease, sizeof (struct event_wmkeyrelease));

    if (handlers->wmkeyrelease)
        handlers->wmkeyrelease(header, &wmkeyrelease);

}

static void onwmmousemove(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmmousemove wmmousemove;

    file_readall(descriptor, &wmmousemove, sizeof (struct event_wmmousemove));

    if (handlers->wmmousemove)
        handlers->wmmousemove(header, &wmmousemove);

}

static void onwmmousepress(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmmousepress wmmousepress;

    file_readall(descriptor, &wmmousepress, sizeof (struct event_wmmousepress));

    if (handlers->wmmousepress)
        handlers->wmmousepress(header, &wmmousepress);

}

static void onwmmouserelease(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header)
{

    struct event_wmmouserelease wmmouserelease;

    file_readall(descriptor, &wmmouserelease, sizeof (struct event_wmmouserelease));

    if (handlers->wmmouserelease)
        handlers->wmmouserelease(header, &wmmouserelease);

}

static void (*funcs[EVENTS])(struct event_handlers *handlers, unsigned int descriptor, struct event_header *header) = {
    0,
    onkeypress,
    onkeyrelease,
    onmousemove,
    onmousepress,
    onmouserelease,
    ontimertick,
    onvideomode,
    onwmmap,
    onwmunmap,
    onwminit,
    onwmexit,
    onwmresize,
    onwmshow,
    onwmhide,
    onwmflush,
    onwmkeypress,
    onwmkeyrelease,
    onwmmousemove,
    onwmmousepress,
    onwmmouserelease
};

unsigned int event_read(struct event_handlers *handlers, unsigned int descriptor)
{

    struct event_header header;
    unsigned int count = file_readall(descriptor, &header, sizeof (struct event_header));

    if (!count)
        return 0;

    if (funcs[header.type])
        funcs[header.type](handlers, descriptor, &header);

    return count;

}

void event_sendwmmap(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMMAP;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmunmap(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMUNMAP;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwminit(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMINIT;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmexit(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMEXIT;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmresize(unsigned int descriptor, unsigned int destination, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int padding, unsigned int lineheight)
{

    struct {struct event_header header; struct event_wmresize wmresize;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMRESIZE;
    message.wmresize.x = x;
    message.wmresize.y = y;
    message.wmresize.w = w;
    message.wmresize.h = h;
    message.wmresize.padding = padding;
    message.wmresize.lineheight = lineheight;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmresize));

}

void event_sendwmshow(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMSHOW;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmhide(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMHIDE;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmflush(unsigned int descriptor, unsigned int destination)
{

    struct event_header header;

    header.destination = destination;
    header.type = EVENT_WMFLUSH;

    file_writeall(descriptor, &header, sizeof (struct event_header));

}

void event_sendwmkeypress(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeypress wmkeypress;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMKEYPRESS;
    message.wmkeypress.scancode = scancode;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmkeypress));

}

void event_sendwmkeyrelease(unsigned int descriptor, unsigned int destination, unsigned char scancode)
{

    struct {struct event_header header; struct event_wmkeyrelease wmkeyrelease;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMKEYRELEASE;
    message.wmkeyrelease.scancode = scancode;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmkeyrelease));

}

void event_sendwmmousemove(unsigned int descriptor, unsigned int destination, char relx, char rely)
{

    struct {struct event_header header; struct event_wmmousemove wmmousemove;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSEMOVE;
    message.wmmousemove.relx = relx;
    message.wmmousemove.rely = rely;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmmousemove));

}

void event_sendwmmousepress(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmousepress wmmousepress;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSEPRESS;
    message.wmmousepress.button = button;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmmousepress));

}

void event_sendwmmouserelease(unsigned int descriptor, unsigned int destination, unsigned int button)
{

    struct {struct event_header header; struct event_wmmouserelease wmmouserelease;} message;

    message.header.destination = destination;
    message.header.type = EVENT_WMMOUSERELEASE;
    message.wmmouserelease.button = button;

    file_writeall(descriptor, &message, sizeof (struct event_header) + sizeof (struct event_wmmouserelease));

}

