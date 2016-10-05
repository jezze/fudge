#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char textdata[FUDGE_BSIZE];
static struct buffer text;
static char outputdata[FUDGE_BSIZE];
static struct buffer output;
static struct box size;
static void (*handlers[EVENTS])(struct event_header *header);

static unsigned int rowstart(unsigned int position)
{

    return ascii_searchreverse(textdata, 0, position, '\n');

}

static unsigned int rowstop(unsigned int position)
{

    return ascii_search(textdata, position, text.count, '\n');

}

static unsigned int rowup(unsigned int position)
{

    unsigned int start;
    unsigned int startp;
    unsigned int count;
    unsigned int countp;

    start = rowstart(position);

    if (!start)
        return 0;

    startp = rowstart(start - 1);
    count = content.cursor - start;
    countp = start - startp - 1;

    return startp + (countp < count ? countp : count);

}

static unsigned int rowdown(unsigned int position)
{

    unsigned int start;
    unsigned int startn;
    unsigned int count;
    unsigned int countn;

    start = rowstart(position);
    startn = rowstop(position) + 1;

    if (startn == text.count)
        return startn - 1;

    count = content.cursor - start;
    countn = rowstop(startn) - startn;

    return startn + (countn < count ? countn : count);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    if (header->source == 0)
        return;

    switch (keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        content.cursor = rowstart(content.cursor);

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    case 0x48:
        content.cursor = rowup(content.cursor);

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    case 0x4B:
        if (content.cursor > 0)
            content.cursor -= 1;

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    case 0x4D:
        if (content.cursor < text.count - 1)
            content.cursor += 1;

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    case 0x4F:
        content.cursor = rowstop(content.cursor);

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    case 0x50:
        content.cursor = rowdown(content.cursor);

        print_text(&output, header->destination, 1, &content, text.memory, text.count);

        break;

    }

}

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(CALL_L0, &keyrelease, sizeof (struct event_keyrelease));

    if (header->source == 0)
        return;

    switch (keyrelease.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmmap(struct event_header *header)
{

}

static void onwmunmap(struct event_header *header)
{

    print_text(&output, header->destination, 0, &content, text.memory, text.count);

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

    print_text(&output, header->destination, 1, &content, text.memory, text.count);

}

static void onwmhide(struct event_header *header)
{

    print_text(&output, header->destination, 0, &content, text.memory, text.count);

}

void main(void)
{

    struct event_header header;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    buffer_init(&output, FUDGE_BSIZE, outputdata);
    buffer_init(&text, FUDGE_BSIZE, textdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        buffer_write(&text, buffer, count);

    file_close(CALL_PI);
    file_open(CALL_PO);
    file_open(CALL_L0);
    file_open(CALL_L1);
    file_open(CALL_L2);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMMAP] = onwmmap;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    send_wmmap(CALL_L1, 0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        if (!handlers[header.type])
            continue;

        handlers[header.type](&header);

        if (output.count)
        {

            file_writeall(CALL_PO, output.memory, output.count);
            buffer_init(&output, FUDGE_BSIZE, outputdata);

        }

        if (quit)
            break;

    }

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

