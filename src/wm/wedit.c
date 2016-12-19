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
static struct ring text;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct box size;
static void (*handlers[EVENTS])(struct event_header *header);

static unsigned int rowleft(unsigned int position)
{

    return (position > 0) ? position - 1 : position;

}

static unsigned int rowright(unsigned int position)
{

    return (position < ring_count(&text) - 1) ? position + 1 : position;

}

static unsigned int rowhome(unsigned int position)
{

    return ascii_searchreverse(textdata, 0, position, '\n');

}

static unsigned int rowend(unsigned int position)
{

    return ascii_search(textdata, position, ring_count(&text), '\n');

}

static unsigned int rowup(unsigned int position)
{

    unsigned int start;
    unsigned int startp;
    unsigned int count;
    unsigned int countp;

    start = rowhome(position);

    if (!start)
        return 0;

    startp = rowhome(start - 1);
    count = position - start;
    countp = start - startp - 1;

    return startp + (countp < count ? countp : count);

}

static unsigned int rowdown(unsigned int position)
{

    unsigned int start;
    unsigned int startn;
    unsigned int count;
    unsigned int countn;

    start = rowhome(position);
    startn = rowend(position) + 1;

    if (startn == ring_count(&text))
        return startn - 1;

    count = position - start;
    countn = rowend(startn) - startn;

    return startn + (countn < count ? countn : count);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;
    struct keycode *keycode;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    if (header->source == 0)
        return;

    switch (keypress.scancode)
    {

    case 0x0E:
        content.cursor = rowleft(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        content.cursor = rowhome(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x48:
        content.cursor = rowup(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x4B:
        content.cursor = rowleft(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x4D:
        content.cursor = rowright(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x4F:
        content.cursor = rowend(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    case 0x50:
        content.cursor = rowdown(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

/*
        if (!ring_write(&input, &keycode->value, keycode->length))
            break;
*/

        memory_copy(textdata + content.cursor, &keycode->value, keycode->length);

        content.cursor = rowright(content.cursor);

        print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
        print_appendtextdata(&output, textdata, ring_count(&text));

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

    print_inserttext(&output, header->destination, &content, 1, ring_count(&text));
    print_appendtextdata(&output, textdata, ring_count(&text));

}

static void onwmhide(struct event_header *header)
{

    print_removetext(&output, header->destination, &content);

}

void main(void)
{

    struct event_header header;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&text, FUDGE_BSIZE, textdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        ring_write(&text, buffer, count);

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

        if (ring_count(&output))
        {

            file_writeall(CALL_PO, outputdata, ring_count(&output));
            ring_reset(&output);

        }

        if (quit)
            break;

    }

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

