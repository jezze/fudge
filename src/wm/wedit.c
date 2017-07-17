#include <abi.h>
#include <fudge.h>
#include <event/event.h>
#include <keymap/keymap.h>
#include <math/box.h>
#include "widget.h"

static struct widget_textbox content;
static struct widget_text status;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static struct event_handlers handlers;
static unsigned int totalrows;
static unsigned int visiblerows;

static void updatecontent(struct event_header *header)
{

    content.length = ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, header->destination, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void updatestatus(struct event_header *header)
{

    status.length = 18;

    widget_update(&output, &status, WIDGET_Z_MIDDLE, header->destination, WIDGET_TYPE_TEXT, sizeof (struct widget_text) + status.length, status.size.x, status.size.y, status.size.w, status.size.h);
    ring_write(&output, &status, sizeof (struct widget_text));
    ring_write(&output, "^S: Save, ^Q: Quit", 18);

}

static void removecontent(struct event_header *header)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, header->destination);

}

static void removestatus(struct event_header *header)
{

    widget_remove(&output, &status, WIDGET_Z_MIDDLE, header->destination);

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

    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    switch (keypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);
        updatecontent(header);

        break;

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x47:
        moveleft(ring_findreverse(&input1, '\n'));
        updatecontent(header);

        break;

    case 0x48:
        moveup();
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
        moveright(ring_find(&input2, '\n'));
        updatecontent(header);

        break;

    case 0x50:
        movedown();
        updatecontent(header);

        break;

    default:
        ring_write(&input1, keycode->value, keycode->length);
        updatecontent(header);

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

static void onwmexit(struct event_header *header)
{

    quit = 1;

}

static void readfile(void)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            ring_write(&input2, &buffer[i], 1);

            if (buffer[i] == '\n')
            {

                totalrows++;

                if (totalrows > visiblerows)
                    return;

            }

        }

    }

    file_close(FILE_PI);

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h - (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&content.size, wmresize->padding);
    box_setsize(&status.size, wmresize->x, wmresize->y + wmresize->h - (wmresize->lineheight + 2 * wmresize->padding), wmresize->w, (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&status.size, wmresize->padding);

    visiblerows = (content.size.h / wmresize->lineheight) - 1;
    totalrows = 0;

    ring_reset(&input1);
    ring_reset(&input2);
    readfile();

}

static void onwmshow(struct event_header *header)
{

    updatecontent(header);
    updatestatus(header);

}

static void onwmhide(struct event_header *header)
{

    removecontent(header);
    removestatus(header);

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
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    widget_inittextbox(&content);
    widget_inittext(&status, WIDGET_TEXTTYPE_HIGHLIGHT);

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/data"))
        return;

    if (!file_walk(FILE_L2, "/system/wm/event"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    file_open(FILE_L2);
    event_sendwmmap(FILE_L2, EVENT_ADDR_BROADCAST);

    while (!quit && event_read(&handlers, FILE_L0))
    {

        if (ring_count(&output))
        {

            file_writeall(FILE_L1, outputdata, ring_count(&output));
            ring_reset(&output);
            event_sendwmflush(FILE_L2, EVENT_ADDR_BROADCAST);

        }

    }

    file_close(FILE_L2);
    file_close(FILE_L1);
    file_close(FILE_L0);

}

