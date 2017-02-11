#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "keymap.h"
#include "ev.h"

static struct element_text content;
static struct element_text status;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static struct ev_handlers handlers;

static void printinsert(unsigned int source)
{

    content.cursor = ring_count(&input1);

    print_inserttext(&output, source, &content, 1, ring_count(&input1) + ring_count(&input2) + 1);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);
    print_inserttext(&output, source, &status, 1, 18);
    ring_write(&output, "^S: Save, ^Q: Quit", 18);

}

static void printremove(unsigned int source)
{

    print_removetext(&output, source, &content, 1);
    print_removetext(&output, source, &status, 1);

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

static void moveup(void)
{

    unsigned int offset1;
    unsigned int offset2;

    offset1 = ring_findreverse(&input1, '\n');

    moveleft(ring_findreverse(&input1, '\n') + 1);

    offset2 = ring_findreverse(&input1, '\n');

    moveleft(offset2 - (offset1 < offset2 ? offset1 : offset2));

}

static void movedown(void)
{

    unsigned int offset1;
    unsigned int offset2;

    offset1 = ring_findreverse(&input1, '\n');

    moveright(ring_find(&input2, '\n') + 1);

    offset2 = ring_find(&input2, '\n');

    moveright(offset1 < offset2 ? offset1 : offset2);

}

static void onkeypress(struct event_header *header, struct event_keypress *keypress)
{

    switch (keypress->scancode)
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
        moveleft(ring_findreverse(&input1, '\n'));
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
        moveright(ring_find(&input2, '\n'));
        printinsert(header->destination);

        break;

    case 0x50:
        movedown();
        printinsert(header->destination);

        break;

    default:
        keymap_write(&input1, keypress->scancode, keymod);
        printinsert(header->destination);

        break;

    }

}

static void onkeyrelease(struct event_header *header, struct event_keyrelease *keyrelease)
{

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmunmap(struct event_header *header)
{

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h - (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&content.size, wmresize->padding);
    box_setsize(&status.size, wmresize->x, wmresize->y + wmresize->h - (wmresize->lineheight + 2 * wmresize->padding), wmresize->w, (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&status.size, wmresize->padding);

}

static void onwmshow(struct event_header *header)
{

    printinsert(header->destination);

}

static void onwmhide(struct event_header *header)
{

    printremove(header->destination);

}

static void readfile(void)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        ring_write(&input2, buffer, count);

}

void main(void)
{

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);
    element_inittext(&status, ELEMENT_TEXTTYPE_HIGHLIGHT, ELEMENT_TEXTFLOW_NORMAL);
    file_open(CALL_PI);
    readfile();
    file_close(CALL_PI);

    if (!file_walk(CALL_L0, "/system/wm/data"))
        return;

    if (!file_walk(CALL_L1, "/system/wm/event"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);
    ev_sendwmmap(CALL_L1, EVENT_ADDR_BROADCAST);

    while (!quit && ev_read(&handlers, CALL_L1))
    {

        char buffer[FUDGE_BSIZE];

        if (!ring_count(&output))
            continue;

        if (file_writeall(CALL_L0, buffer, ring_read(&output, buffer, FUDGE_BSIZE)))
            ev_sendwmflush(CALL_L1, EVENT_ADDR_BROADCAST);

    }

    file_close(CALL_L1);
    file_close(CALL_L0);

}

