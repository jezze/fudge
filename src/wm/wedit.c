#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char textbuffer[FUDGE_BSIZE];
static unsigned int textcount;
static struct box size;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct event_header *header);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, void *buffer, unsigned int count)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static unsigned int rowstart()
{

    return ascii_searchreverse(textbuffer, 0, content.cursor, '\n');

}

static unsigned int rowstop()
{

    return ascii_search(textbuffer, content.cursor, textcount, '\n');

}

static unsigned int rowup()
{

    unsigned int start;
    unsigned int startp;
    unsigned int count;
    unsigned int countp;

    start = ascii_searchreverse(textbuffer, 0, content.cursor, '\n');

    if (!start)
        return 0;

    startp = ascii_searchreverse(textbuffer, 0, start - 1, '\n');
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

    start = ascii_searchreverse(textbuffer, 0, content.cursor, '\n');
    startn = ascii_search(textbuffer, content.cursor, textcount, '\n') + 1;

    if (startn == textcount)
        return startn - 1;

    count = content.cursor - start;
    countn = ascii_search(textbuffer, startn, textcount, '\n') - startn;

    return startn + (countn < count ? countn : count);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    switch (keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        content.cursor = rowstart();

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x48:
        content.cursor = rowup();

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x4B:
        if (content.cursor > 0)
            content.cursor -= 1;

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x4D:
        if (content.cursor < textcount - 1)
            content.cursor += 1;

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x4F:
        content.cursor = rowstop();

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x50:
        content.cursor = rowdown();

        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    }

}

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(CALL_L0, &keyrelease, sizeof (struct event_keyrelease));

    switch (keyrelease.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmunmap(struct event_header *header)
{

    writetext(header->destination, 0, &content, textbuffer, textcount);

    quit = 1;

}

static void onwmresize(struct event_header *header)
{

    struct event_wmresize wmresize;

    file_readall(CALL_L0, &wmresize, sizeof (struct event_wmresize));
    box_setsize(&size, wmresize.x, wmresize.y, wmresize.w, wmresize.h);
    box_setsize(&content.size, size.x + 12, size.y + 12, size.w - 24, size.h - 24);

}

static void onwmshow(struct event_header *header)
{

    writetext(header->destination, 1, &content, textbuffer, textcount);

}

static void onwmhide(struct event_header *header)
{

    writetext(header->destination, 0, &content, textbuffer, textcount);

}

static void setup(void)
{

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    textcount = file_read(CALL_PI, textbuffer, FUDGE_BSIZE);

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

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    file_open(CALL_PO);
    file_open(CALL_L0);
    file_open(CALL_L1);
    send_wmmap(CALL_L1, 0, 0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        if (!handlers[header.type])
            continue;

        handlers[header.type](&header);

        if (datacount)
        {

            file_writeall(CALL_PO, databuffer, datacount);

            datacount = 0;

        }

        if (quit)
            break;

    }

    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

