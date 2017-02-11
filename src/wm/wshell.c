#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "keymap.h"
#include "ev.h"

static struct element_text content;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE >> 5];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE >> 5];
static struct ring input2;
static char textdata[FUDGE_BSIZE];
static struct ring text;
static struct ev_handlers handlers;
static unsigned int totalrows = 0;
static unsigned int visiblerows = 0;

static void printinsert(unsigned int source)
{

    content.cursor = ring_count(&text) + ring_count(&input1);

    print_inserttext(&output, source, &content, 1, ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1);
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void printremove(unsigned int source)
{

    print_removetext(&output, source, &content, 1);

}

static void removerows(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        ring_skip(&text, ring_find(&text, '\n') + 1);

        totalrows--;

    }

}

static void copychar(char c)
{

    ring_write(&text, &c, 1);

    if (c == '\n')
    {

        totalrows++;

        if (totalrows > visiblerows)
            removerows(totalrows - visiblerows);

    }

}

static void copybuffer(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        copychar(b[i]);

}

static void copyring(struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;

    while ((count = ring_read(ring, buffer, FUDGE_BSIZE)))
        copybuffer(buffer, count);

    ring->head = head;
    ring->tail = tail;

}

static void readback(void)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(CALL_CO);

    while ((count = file_read(CALL_CO, buffer, FUDGE_BSIZE)))
        copybuffer(buffer, count);

    file_close(CALL_CO);

}

static void interpret(struct ring *ring)
{

    char command[FUDGE_BSIZE >> 5];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE >> 5);

    if (count < 2)
        return;

    /* This is a temporary fix */
    if (memory_match(command, "cd ", 3))
    {

        if (count < 4)
            return;

        command[count - 1] = '\0';

        if (file_walk(CALL_L8, command + 3))
        {

            file_duplicate(CALL_PW, CALL_L8);
            file_duplicate(CALL_CW, CALL_L8);

        }

        return;

    }

    if (!file_walk(CALL_CP, "/bin/slang"))
        return;

    if (!file_walk(CALL_L8, "/system/pipe/clone/"))
        return;

    file_walkfrom(CALL_CI, CALL_L8, "0");
    file_walkfrom(CALL_CO, CALL_L8, "1");
    file_open(CALL_CI);
    file_writeall(CALL_CI, command, count);
    call_spawn();
    file_close(CALL_CI);
    readback();

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

static void onkeypress(struct event_header *header, struct event_keypress *keypress)
{

    switch (keypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);
        printinsert(header->destination);

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        keymap_write(&input1, keypress->scancode, keymod);
        copyring(&input1);
        interpret(&input1);
        copybuffer("$ ", 2);
        printinsert(header->destination);

        break;

    case 0x1D:
        keymod |= KEYMOD_CTRL;

        break;

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x16:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            keymap_write(&input1, keypress->scancode, keymod);

        printinsert(header->destination);

        break;

    case 0x25:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            keymap_write(&input1, keypress->scancode, keymod);

        printinsert(header->destination);

        break;

    case 0x26:
        if (keymod & KEYMOD_CTRL)
            removerows(totalrows);
        else
            keymap_write(&input1, keypress->scancode, keymod);

        printinsert(header->destination);

        break;

    case 0x47:
        moveleft(ring_count(&input1));
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
        moveright(ring_count(&input2));
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

    case 0x1D:
        keymod &= ~KEYMOD_CTRL;

        break;

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

    unsigned int padding = wmresize->factor + 2;

    visiblerows = ((wmresize->h - padding * 2) / wmresize->lineheight) - 1;

    if (totalrows > visiblerows)
        removerows(totalrows - visiblerows);

    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_resize(&content.size, padding);

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

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE >> 5, inputdata1);
    ring_init(&input2, FUDGE_BSIZE >> 5, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);
    copybuffer("$ ", 2);

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

