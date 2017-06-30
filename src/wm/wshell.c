#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "widget.h"
#include "keymap.h"
#include "ev.h"

static struct widget_textbox content;
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
static unsigned int totalrows;
static unsigned int visiblerows;

static void updatecontent(struct event_header *header)
{

    content.length = ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, header->destination, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void removecontent(struct event_header *header)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, header->destination);

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

static void copybuffer(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        ring_write(&text, &b[i], 1);

        if (b[i] == '\n')
        {

            totalrows++;

            if (totalrows > visiblerows)
                removerows(totalrows - visiblerows);

        }

    }

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

    file_open(FILE_CO);

    while ((count = file_read(FILE_CO, buffer, FUDGE_BSIZE)))
        copybuffer(buffer, count);

    file_close(FILE_CO);

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

        if (file_walk(FILE_L8, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L8);
            file_duplicate(FILE_CW, FILE_L8);

        }

        return;

    }

    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk(FILE_L8, "/system/pipe/clone/"))
        return;

    file_walkfrom(FILE_CI, FILE_L8, "0");
    file_walkfrom(FILE_CO, FILE_L8, "1");
    file_open(FILE_CI);
    file_writeall(FILE_CI, command, count);
    call_spawn();
    file_close(FILE_CI);
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
        updatecontent(header);

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        keymap_write(&input1, keypress->scancode, keymod);
        copyring(&input1);
        interpret(&input1);
        copybuffer("$ ", 2);
        updatecontent(header);

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

        updatecontent(header);

        break;

    case 0x25:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            keymap_write(&input1, keypress->scancode, keymod);

        updatecontent(header);

        break;

    case 0x26:
        if (keymod & KEYMOD_CTRL)
            removerows(totalrows);
        else
            keymap_write(&input1, keypress->scancode, keymod);

        updatecontent(header);

        break;

    case 0x47:
        moveleft(ring_count(&input1));
        updatecontent(header);

        break;

    case 0x4B:
        moveleft(1);
        updatecontent(header);

        break;

    case 0x4D:
        moveright(1);
        updatecontent(header);

        break;

    case 0x4F:
        moveright(ring_count(&input2));
        updatecontent(header);

        break;

    default:
        keymap_write(&input1, keypress->scancode, keymod);
        updatecontent(header);

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

static void onwmexit(struct event_header *header)
{

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_resize(&content.size, wmresize->padding);

    visiblerows = (content.size.h / wmresize->lineheight) - 1;

    if (totalrows > visiblerows)
        removerows(totalrows - visiblerows);

}

static void onwmshow(struct event_header *header)
{

    updatecontent(header);

}

static void onwmhide(struct event_header *header)
{

    removecontent(header);

}

void main(void)
{

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.wmexit = onwmexit;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE >> 5, inputdata1);
    ring_init(&input2, FUDGE_BSIZE >> 5, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    widget_inittextbox(&content);
    copybuffer("$ ", 2);

    if (!file_walk(FILE_L0, "/system/wm/data"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    ev_sendwmmap(FILE_L1, EVENT_ADDR_BROADCAST);

    while (!quit && ev_read(&handlers, FILE_L1))
    {

        if (ring_count(&output))
        {

            file_writeall(FILE_L0, outputdata, ring_count(&output));
            ring_reset(&output);
            ev_sendwmflush(FILE_L1, EVENT_ADDR_BROADCAST);

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

