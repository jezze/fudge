#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "box.h"
#include "element.h"
#include "send.h"

static struct element_text content;
static unsigned int quit;
static unsigned char text[FUDGE_BSIZE];
static unsigned int textcount;
static struct box size;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct event_header *header, void *data);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, unsigned int count, void *buffer)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void onwmunmap(struct event_header *header, void *data)
{

    writetext(header->destination, 0, &content, textcount, text);

    quit = 1;

}

static void onwmresize(struct event_header *header, void *data)
{

    struct event_wmresize *wmresize = data;

    box_setsize(&size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&content.size, size.x + 12, size.y + 12, size.w - 24, size.h - 24);

}

static void onwmshow(struct event_header *header, void *data)
{

    writetext(header->destination, 1, &content, textcount, text);

}

static void onwmhide(struct event_header *header, void *data)
{

    writetext(header->destination, 0, &content, textcount, text);

}

static void setup(void)
{

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL);

    quit = 0;
    textcount = file_read(CALL_PI, text, FUDGE_BSIZE);

}

void main(void)
{

    struct event_header header;
    unsigned int count;

    setup();

    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    if (!call_walk(CALL_L0, CALL_PR, 17, "system/event/poll"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);
    send_wmmap(CALL_L0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        unsigned char data[512];

        if (header.count)
            file_readall(CALL_L0, data, header.count);

        if (handlers[header.type])
            handlers[header.type](&header, data);

        if (datacount)
        {

            file_writeall(CALL_PO, databuffer, datacount);

            datacount = 0;

        }

        if (quit)
            break;

    }

    call_close(CALL_L0);
    call_close(CALL_PO);

}

