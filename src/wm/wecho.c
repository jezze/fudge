#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char text[FUDGE_BSIZE];
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

static unsigned int rowup()
{

    unsigned int start;
    unsigned int startp;
    unsigned int count;
    unsigned int countp;

    start = ascii_searchreverse(text, 0, content.cursor, '\n');

    if (!start)
        return 0;

    startp = ascii_searchreverse(text, 0, start - 1, '\n');
    count = content.cursor - start;
    countp = start - startp - 1;

    return startp + (countp < count ? countp : count);

}

static unsigned int rowdown()
{

    unsigned int start;
    unsigned int startn;
    unsigned int count;
    unsigned int countn;

    start = ascii_searchreverse(text, 0, content.cursor, '\n');
    startn = ascii_search(text, content.cursor, textcount, '\n') + 1;

    if (startn == textcount)
        return startn - 1;

    count = content.cursor - start;
    countn = ascii_search(text, startn, textcount, '\n') - startn;

    return startn + (countn < count ? countn : count);

}

static void onkeypress(struct event_header *header, void *data)
{

    struct event_keypress *keypress = data;

    switch (keypress->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x11:
        content.cursor = rowup();

        writetext(header->destination, 1, &content, textcount, text);

        break;

    case 0x1E:
        if (content.cursor > 0)
            content.cursor -= 1;

        writetext(header->destination, 1, &content, textcount, text);

        break;

    case 0x1F:
        content.cursor = rowdown();

        writetext(header->destination, 1, &content, textcount, text);

        break;

    case 0x20:
        if (content.cursor < textcount - 1)
            content.cursor += 1;

        writetext(header->destination, 1, &content, textcount, text);

        break;

    }

}

static void onkeyrelease(struct event_header *header, void *data)
{

    struct event_keyrelease *keyrelease = data;

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

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

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    textcount = file_read(CALL_PI, text, FUDGE_BSIZE);

}

void main(void)
{

    struct event_header header;
    unsigned int count;

    setup();

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
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

