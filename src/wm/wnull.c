#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static struct box screen;
static unsigned int quit;
static void (*handlers[EVENTS])(union event *event);
static unsigned char text[FUDGE_BSIZE];
static unsigned int textcount;
static unsigned int modifier;

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

static void flush(void)
{

    if (datacount)
    {

        call_write(CALL_PO, datacount, databuffer);

        datacount = 0;

    }

}

static void onkeypress(union event *event)
{

    struct keycode *keycode;

    switch (event->keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        modifier |= KEYMOD_SHIFT;

        break;

    case 0x38:
        modifier |= KEYMOD_ALT;

        break;

    case 0x0E:
        if (textcount)
        {

            textcount -= 1;

            writetext(event->header.destination, 1, &content, textcount, text);

        }

        break;

    default:
        keycode = getkeycode(KEYMAP_US, event->keypress.scancode, modifier);
        textcount += memory_write(text, FUDGE_BSIZE, keycode->value, keycode->length, textcount);

        writetext(event->header.destination, 1, &content, textcount, text);

        break;
    }

}

static void onkeyrelease(union event *event)
{

    switch (event->keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        modifier &= ~KEYMOD_SHIFT;

        break;

    case 0x38:
        modifier &= ~KEYMOD_ALT;

        break;

    }

}

static void onwmunmap(union event *event)
{

    writetext(event->header.destination, 0, &content, textcount, text);

    quit = 1;

}

static void onwmresize(union event *event)
{

    box_setsize(&screen, event->wmresize.x, event->wmresize.y, event->wmresize.w, event->wmresize.h);
    box_setsize(&content.size, screen.x + 12, screen.y + 12, screen.w - 24, screen.h - 24);

}

static void onwmshow(union event *event)
{

    writetext(event->header.destination, 1, &content, textcount, text);

}

static void onwmhide(union event *event)
{

    writetext(event->header.destination, 0, &content, textcount, text);

}

void main(void)
{

    union event event;
    unsigned int count;

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    call_open(CALL_PO);
    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);
    send_wmmap(CALL_L2, 0);

    while ((count = call_read(CALL_L1, sizeof (struct event_header), &event.header)))
    {

        if (event.header.count)
            call_read(CALL_L1, event.header.count, event.data + sizeof (struct event_header));

        if (handlers[event.header.type])
        {

            handlers[event.header.type](&event);
            flush();

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

