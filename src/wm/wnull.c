#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"

#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2

struct keycode
{

    unsigned char length;
    unsigned char value[4];

};

struct keyset
{

    struct keycode keycode[4];

};

static struct element_text content;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static struct box screen;
static unsigned int quit;
static void (*handlers[EVENTS])(union event *event);
static unsigned char text[256];
static unsigned int textcount;
static struct keyset map[256] = {
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x31, 0x00, 0x00, 0x00}}, {1, {0x21, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x32, 0x00, 0x00, 0x00}}, {1, {0x40, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x33, 0x00, 0x00, 0x00}}, {1, {0x23, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x34, 0x00, 0x00, 0x00}}, {1, {0x24, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x35, 0x00, 0x00, 0x00}}, {1, {0x25, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x36, 0x00, 0x00, 0x00}}, {1, {0x5E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x37, 0x00, 0x00, 0x00}}, {1, {0x26, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x38, 0x00, 0x00, 0x00}}, {1, {0x2A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x39, 0x00, 0x00, 0x00}}, {1, {0x28, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x30, 0x00, 0x00, 0x00}}, {1, {0x29, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2D, 0x00, 0x00, 0x00}}, {1, {0x5F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2B, 0x00, 0x00, 0x00}}, {1, {0x3D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x08, 0x00, 0x00, 0x00}}, {1, {0x08, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x09, 0x00, 0x00, 0x00}}, {1, {0x09, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x71, 0x00, 0x00, 0x00}}, {1, {0x51, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x77, 0x00, 0x00, 0x00}}, {1, {0x57, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x65, 0x00, 0x00, 0x00}}, {1, {0x45, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x72, 0x00, 0x00, 0x00}}, {1, {0x52, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x74, 0x00, 0x00, 0x00}}, {1, {0x54, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x79, 0x00, 0x00, 0x00}}, {1, {0x59, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x75, 0x00, 0x00, 0x00}}, {1, {0x55, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x69, 0x00, 0x00, 0x00}}, {1, {0x49, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6F, 0x00, 0x00, 0x00}}, {1, {0x4F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x70, 0x00, 0x00, 0x00}}, {1, {0x50, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5B, 0x00, 0x00, 0x00}}, {1, {0x7B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5D, 0x00, 0x00, 0x00}}, {1, {0x7D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x0A, 0x00, 0x00, 0x00}}, {1, {0x0A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x61, 0x00, 0x00, 0x00}}, {1, {0x41, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x73, 0x00, 0x00, 0x00}}, {1, {0x53, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x64, 0x00, 0x00, 0x00}}, {1, {0x44, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x66, 0x00, 0x00, 0x00}}, {1, {0x46, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x67, 0x00, 0x00, 0x00}}, {1, {0x47, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x68, 0x00, 0x00, 0x00}}, {1, {0x48, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6A, 0x00, 0x00, 0x00}}, {1, {0x4A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6B, 0x00, 0x00, 0x00}}, {1, {0x4B, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6C, 0x00, 0x00, 0x00}}, {1, {0x4C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x3B, 0x00, 0x00, 0x00}}, {1, {0x3A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x27, 0x00, 0x00, 0x00}}, {1, {0x22, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x60, 0x00, 0x00, 0x00}}, {1, {0x7E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x5C, 0x00, 0x00, 0x00}}, {1, {0x7C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x7A, 0x00, 0x00, 0x00}}, {1, {0x5A, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x78, 0x00, 0x00, 0x00}}, {1, {0x58, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x63, 0x00, 0x00, 0x00}}, {1, {0x43, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x76, 0x00, 0x00, 0x00}}, {1, {0x56, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x62, 0x00, 0x00, 0x00}}, {1, {0x42, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6E, 0x00, 0x00, 0x00}}, {1, {0x4E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x6D, 0x00, 0x00, 0x00}}, {1, {0x4D, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2C, 0x00, 0x00, 0x00}}, {1, {0x3C, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2E, 0x00, 0x00, 0x00}}, {1, {0x3E, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x2F, 0x00, 0x00, 0x00}}, {1, {0x3F, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
    {{{1, {0x20, 0x00, 0x00, 0x00}}, {1, {0x20, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}, {0, {0x00, 0x00, 0x00, 0x00}}}},
};

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

        return;

    case 0x38:
        modifier |= KEYMOD_ALT;

        return;

    }

    keycode = &map[event->keypress.scancode].keycode[modifier];
    textcount += memory_write(text, 256, keycode->value, keycode->length, textcount);

    writetext(event->header.destination, 1, &content, textcount, text);

}

static void onkeyrelease(union event *event)
{

    switch (event->keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        modifier &= ~KEYMOD_SHIFT;

        return;

    case 0x38:
        modifier &= ~KEYMOD_ALT;

        return;

    }

}

static void onwmunmap(union event *event)
{

    quit = 1;

}

static void onwmresize(union event *event)
{

    box_setsize(&screen, event->wmresize.x, event->wmresize.y, event->wmresize.w, event->wmresize.h);
    box_setsize(&content.size, screen.x + 8, screen.y + 8, screen.w - 16, 18);
    writetext(event->header.destination, 1, &content, textcount, text);

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

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL);
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

