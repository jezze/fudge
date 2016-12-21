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
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static void (*handlers[EVENTS])(struct event_header *header);

static void printinsert(unsigned int source)
{

    content.cursor = ring_count(&input1);

    print_inserttext(&output, source, &content, 1, ring_count(&input1) + ring_count(&input2) + 1);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void printremove(unsigned int source)
{

    print_removetext(&output, source, &content);

}

static void moveleft(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[FUDGE_BSIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void movehome()
{

    moveleft(ring_findreverse(&input1, '\n'));

}

static void moveend()
{

    moveright(ring_find(&input2, '\n'));

}

static void moveup()
{

    unsigned int offset1;
    unsigned int offset2;

    offset1 = ring_findreverse(&input1, '\n');

    movehome();

    if (!ring_count(&input1))
        return;

    moveleft(1);

    if (!ring_count(&input1))
        return;

    offset2 = ring_findreverse(&input1, '\n');

    moveleft(offset2 - (offset1 < offset2 ? offset1 : offset2));

}

static void movedown()
{

    unsigned int offset1;
    unsigned int offset2;

    offset1 = ring_findreverse(&input1, '\n');

    moveend();

    if (!ring_count(&input2))
        return;

    moveright(1);

    if (!ring_count(&input2))
        return;

    offset2 = ring_find(&input2, '\n');

    moveright(offset1 < offset2 ? offset1 : offset2);

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
        ring_skipreverse(&input1, 1);
        printinsert(header->destination);

        break;

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        movehome();
        printinsert(header->destination);

        break;

    case 0x48:
        moveup();
        printinsert(header->destination);

        break;

    case 0x4B:
        moveleft(1);
        printinsert(header->destination);

        break;

    case 0x4D:
        moveright(1);
        printinsert(header->destination);

        break;

    case 0x4F:
        moveend(1);
        printinsert(header->destination);

        break;

    case 0x50:
        movedown();
        printinsert(header->destination);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        ring_write(&input1, &keycode->value, keycode->length);
        printinsert(header->destination);

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
    box_setsize(&content.size, wmresize.x + 12, wmresize.y + 12, wmresize.w - 24, wmresize.h - 24);

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

    struct event_header header;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

    file_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        ring_write(&input2, buffer, count);

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

            char buffer[FUDGE_BSIZE];

            file_writeall(CALL_PO, buffer, ring_read(&output, buffer, FUDGE_BSIZE));

        }

        if (quit)
            break;

    }

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

