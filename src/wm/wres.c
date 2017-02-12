#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "ev.h"
#include "render.h"

static unsigned int quit;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct ev_handlers handlers;

static void printinsert(unsigned int source)
{

}

static void printremove(unsigned int source)
{

}

static void onmousepress(struct event_header *header, struct event_mousepress *mousepress)
{

    switch (mousepress->button)
    {

    case 0x01:
        render_init();
        render_initvideo(CALL_L6, 1920, 1080, 32);

        break;

    }

}

static void onwmunmap(struct event_header *header)
{

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

}

static void onwmshow(struct event_header *header)
{

    printinsert(header->destination);

}

static void onwmhide(struct event_header *header)
{

    printremove(header->destination);

}

void main(void)
{

    handlers.mousepress = onmousepress;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);

    if (!file_walk(CALL_L0, "/system/wm/data"))
        return;

    if (!file_walk(CALL_L1, "/system/wm/event"))
        return;

    if (!file_walk(CALL_L6, "/system/video/if:0/ctrl"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);
    ev_sendwmmap(CALL_L1, EVENT_ADDR_BROADCAST);

    while (!quit && ev_read(&handlers, CALL_L1))
    {

        char buffer[FUDGE_BSIZE];

        if (!ring_count(&output))
            continue;

        if (file_writeall(CALL_L0, buffer, ring_read(&output, buffer, FUDGE_BSIZE)))
            ev_sendwmflush(CALL_L1, EVENT_ADDR_BROADCAST);

    }

    file_close(CALL_L1);
    file_close(CALL_L0);

}

