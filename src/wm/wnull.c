#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"

static struct element_text content;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static struct box screen;
static unsigned int source;
static unsigned int quit;
static void (*handlers[EVENTS])(union event *event);

static void writetext(unsigned int source, struct element_text *text, unsigned int count, void *buffer)
{

    text->base.source = source;
    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void flush()
{

    if (datacount)
    {

        call_write(CALL_PO, datacount, databuffer);

        datacount = 0;

    }

}

static void onwmmapnotify(union event *event)
{

    source = event->header.destination;

    box_setsize(&screen, event->wmmapnotify.x, event->wmmapnotify.y, event->wmmapnotify.w, event->wmmapnotify.h);

    content.base.size.x = screen.x + 8;
    content.base.size.y = screen.y + 8;

}

static void onwmexpose(union event *event)
{

    writetext(source, &content, 6, "Hello!");
    flush();

}

static void onwmunmap(union event *event)
{

    quit = 1;

}

void main(void)
{

    union event event;
    unsigned int count;

    handlers[EVENT_WMMAPNOTIFY] = onwmmapnotify;
    handlers[EVENT_WMEXPOSE] = onwmexpose;
    handlers[EVENT_WMUNMAP] = onwmunmap;

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL);

    content.base.count += 6;
    content.base.size.h = 16;
    content.base.size.w = 64;

    call_open(CALL_PO);
    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);
    send_wmmap(CALL_L2, 0);

    while ((count = call_read(CALL_L1, sizeof (struct event_header), &event.header)))
    {

        if (event.header.count)
            call_read(CALL_L1, event.header.count, event.data + sizeof (struct event_header));

        if (handlers[event.header.type])
            handlers[event.header.type](&event);

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

