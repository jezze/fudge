#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "text.h"
#include "send.h"

static struct text content;
static struct list renderables;
static struct box screen;

static unsigned int writerenderable(unsigned int source, void *buffer, unsigned int count, struct renderable *renderable)
{

    renderable->header.source = source;

    return memory_write(buffer, FUDGE_BSIZE, &renderable->header, sizeof (struct renderable_header), count);

}

static unsigned int writetext(void *buffer, unsigned int count, struct text *text)
{

    return memory_write(buffer, FUDGE_BSIZE, &text->header, sizeof (struct text_header), count);

}

static unsigned int writepayload(void *buffer, unsigned int count, void *payload, unsigned int payloadcount)
{

    return memory_write(buffer, FUDGE_BSIZE, payload, payloadcount, count);

}

static void render(unsigned int source, struct box *bb)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = 0;
    struct list_item *current;

    for (current = renderables.head; current; current = current->next)
    {

        struct renderable *renderable = current->data;
        struct text *text;

        if (!renderable->modified)
            continue;

        count += writerenderable(source, buffer, count, renderable);

        switch (renderable->header.type)
        {

        case RENDERABLE_TYPE_TEXT:
            text = current->data;

            count += writetext(buffer, count, text);
            count += writepayload(buffer, count, text->string, text->count);

            break;

        }

        content.base.modified = 0;

    }

    call_write(CALL_PO, count, buffer);

}

void main(void)
{

    union event event;
    unsigned int count, quit = 0;
    unsigned int source = 0;

    text_init(&content, TEXT_TYPE_NORMAL);
    text_assign(&content, 6, "Hello!");

    content.base.modified = 1;
    content.base.header.size.h = 16;
    content.base.header.size.w = 64;

    list_add(&renderables, &content.base.item);
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

        case EVENT_WMREADY:
            source = event.header.destination;

            box_setsize(&screen, event.wmready.x, event.wmready.y, event.wmready.w, event.wmready.h);

            content.base.modified = 1;
            content.base.header.size.x = screen.x + 8;
            content.base.header.size.y = screen.y + 8;

            break;

        case EVENT_WMEXPOSE:
            content.base.modified = 1;

            render(source, &screen);

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

