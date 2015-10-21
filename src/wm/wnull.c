#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "text.h"
#include "send.h"

static struct text content;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;

static void writerenderable(unsigned int source, struct renderable *renderable)
{

    renderable->source = source;

    datacount += memory_write(databuffer, FUDGE_BSIZE, renderable, sizeof (struct renderable), datacount);

}

static void writetext(unsigned int source, struct text *text, unsigned int count, void *buffer)
{

    writerenderable(source, &text->base);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &text->header, sizeof (struct text_header), datacount);
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

void main(void)
{

    union event event;
    unsigned int count, quit = 0;
    unsigned int source = 0;
    struct box screen;

    text_init(&content, TEXT_TYPE_NORMAL);

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

        switch (event.header.type)
        {

        case EVENT_WMMAPNOTIFY:
            source = event.header.destination;

            box_setsize(&screen, event.wmmapnotify.x, event.wmmapnotify.y, event.wmmapnotify.w, event.wmmapnotify.h);

            content.base.size.x = screen.x + 8;
            content.base.size.y = screen.y + 8;

            break;

        case EVENT_WMEXPOSE:
            writetext(source, &content, 6, "Hello!");
            flush();

            break;
            
        case EVENT_WMUNMAP:
            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

