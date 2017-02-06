#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "keymap.h"
#include "ev.h"

#define ROWS 32
#define COLS 128

struct row
{

    char data[COLS];
    struct ring input;
    struct element_text content;

};

static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct ev_handlers handlers;
static struct row rows[ROWS];
static unsigned int startrow = 0;
static unsigned int currentrow = 0;
static unsigned int visiblerows = ROWS;
static unsigned int lastrow = 0;

static void printinsert(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < ROWS; i++)
    {

        print_inserttext(&output, source, &rows[i].content, 1, ring_count(&rows[i].input));
        ring_copy(&output, &rows[i].input);

    }

}

static void printremove(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < ROWS; i++)
        print_removetext(&output, source, &rows[i].content);

}

static void changerow(unsigned int row)
{

    rows[currentrow].content.flow = ELEMENT_TEXTFLOW_NORMAL;
    rows[row].content.cursor = (rows[currentrow].content.cursor < ring_count(&rows[row].input)) ? rows[currentrow].content.cursor : ring_count(&rows[row].input) - 1;
    rows[row].content.flow = ELEMENT_TEXTFLOW_INPUT;
    currentrow = row;

}

static void movehome(void)
{

    rows[currentrow].content.cursor = 0;

}

static void moveend(void)
{

    rows[currentrow].content.cursor = ring_count(&rows[currentrow].input) - 1;

}

static void moveup(void)
{

    if (currentrow > 0)
        changerow(currentrow - 1);
    else
        movehome();

}

static void movedown(void)
{

    if (currentrow < lastrow - 1)
        changerow(currentrow + 1);
    else
        moveend();

}

static void moveleft(void)
{

    if (rows[currentrow].content.cursor > 0)
    {

        rows[currentrow].content.cursor--;

    }

    else
    {

        moveup();
        moveend();

    }

}

static void moveright(void)
{

    if (rows[currentrow].content.cursor < ring_count(&rows[currentrow].input) - 1)
    {

        rows[currentrow].content.cursor++;

    }

    else
    {

        movedown();
        movehome();

    }

}

static void reset(void)
{

    unsigned int i;

    for (i = 0; i < ROWS; i++)
    {

        rows[i].input.head = 0;
        rows[i].input.tail = 0;

    }

}

static void skipline(void)
{

    char c;

    while (file_read(CALL_PI, &c, 1))
    {

        if (c == '\n')
            break;

    }

}

static void readline(struct ring *ring)
{

    char c;

    while (file_read(CALL_PI, &c, 1))
    {

        ring_write(ring, &c, 1);

        if (c == '\n')
            break;

    }

}

static unsigned int readfile(unsigned int maxrows)
{

    unsigned int row;

    for (row = 0; row < startrow; row++)
        skipline();

    for (row = 0; row < maxrows; row++)
    {

        readline(&rows[row].input);

        if (!ring_count(&rows[row].input))
            break;

    }

    return row;

}

static void readall(void)
{

    unsigned int rowcount;

    reset();
    file_open(CALL_PI);

    rowcount = readfile(visiblerows);
    lastrow = (rowcount < visiblerows) ? rowcount : visiblerows;

    file_close(CALL_PI);

}

static void onkeypress(struct event_header *header, struct event_keypress *keypress)
{

    switch (keypress->scancode)
    {

    case 0x0E:
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

    case 0x49:
        if (startrow > 0)
            startrow--;

        readall();
        printinsert(header->destination);

        break;

    case 0x4B:
        moveleft();
        printinsert(header->destination);

        break;

    case 0x4D:
        moveright();
        printinsert(header->destination);

        break;

    case 0x4F:
        moveend();
        printinsert(header->destination);

        break;

    case 0x50:
        movedown();
        printinsert(header->destination);

        break;

    case 0x51:
        if (lastrow > 1)
            startrow++;

        readall();
        printinsert(header->destination);

        break;

    default:
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

    unsigned int i;

    visiblerows = (wmresize->h - 24) / 24;

    if (visiblerows > ROWS)
        visiblerows = ROWS;

    for (i = 0; i < ROWS; i++)
        box_setsize(&rows[i].content.size, wmresize->x + 12, wmresize->y + 12 + i * 24, wmresize->w - 24, 24);

    readall();

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

    unsigned int i;

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);

    for (i = 0; i < ROWS; i++)
    {

        ring_init(&rows[i].input, COLS, rows[i].data);
        element_inittext(&rows[i].content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_NORMAL);

    }

    rows[currentrow].content.flow = ELEMENT_TEXTFLOW_INPUT;

    if (!file_walk(CALL_L0, "/system/wm/send"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);
    ev_sendwmmap(CALL_L1, EVENT_ADDR_BROADCAST);

    while (!quit && ev_read(&handlers, CALL_L1))
        ev_sendwmflush(CALL_L1, EVENT_ADDR_BROADCAST, CALL_L0, &output);

    file_close(CALL_L1);
    file_close(CALL_L0);

}

