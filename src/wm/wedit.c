#include <abi.h>
#include <fudge.h>
#include <event/base.h>
#include <event/wm.h>
#include <keymap/keymap.h>
#include <math/box.h>
#include <widget/widget.h>

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

static void updatecontent(struct event_header *header)
{

    content.length = ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, header->target, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void updatestatus(struct event_header *header)
{

    status.length = 18;

    widget_update(&output, &status, WIDGET_Z_MIDDLE, header->target, WIDGET_TYPE_TEXT, sizeof (struct widget_text) + status.length, status.size.x, status.size.y, status.size.w, status.size.h);
    ring_write(&output, &status, sizeof (struct widget_text));
    ring_write(&output, "^S: Save, ^Q: Quit", 18);

}

static void removecontent(struct event_header *header)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, header->target);

}

static void removestatus(struct event_header *header)
{

    widget_remove(&output, &status, WIDGET_Z_MIDDLE, header->target);

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

static void oninit(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    widget_inittextbox(&content);
    widget_inittext(&status, WIDGET_TEXTTYPE_HIGHLIGHT);
    event_addrequest(message, header, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_sendbuffer(FILE_L0, message);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addrequest(message, header, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_sendbuffer(FILE_L0, message);
    event_addresponse(message, header, EVENT_CHILD);
    event_sendbuffer(FILE_L0, message);

    quit = 1;

}

static void onfile(struct event_header *header)
{

}

static void onwmkeypress(struct event_header *header)
{

    struct event_wmkeypress *wmkeypress = event_getdata(header);
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, wmkeypress->scancode, keymod);

    keymod = keymap_modkey(wmkeypress->scancode, keymod);

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);
        updatecontent(header);

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

static void onwmkeyrelease(struct event_header *header)
{

    struct event_wmkeyrelease *wmkeyrelease = event_getdata(header);

    keymod = keymap_modkey(wmkeyrelease->scancode, keymod);

}

static unsigned int readfile(unsigned int descriptor, unsigned int visiblerows)
{

    char buffer[FUDGE_BSIZE];
    unsigned int rows = 0;
    unsigned int count;

    while ((count = file_read(descriptor, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            ring_write(&input2, &buffer[i], 1);

            if (buffer[i] == '\n')
            {

                rows++;

                if (rows >= visiblerows)
                    return rows;

            }

        }

    }

    return rows;

}

static void onwmresize(struct event_header *header)
{

    struct event_wmresize *wmresize = event_getdata(header);

    ring_reset(&input1);
    ring_reset(&input2);
    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h - (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&content.size, wmresize->padding);
    box_setsize(&status.size, wmresize->x, wmresize->y + wmresize->h - (wmresize->lineheight + 2 * wmresize->padding), wmresize->w, (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&status.size, wmresize->padding);
    file_open(FILE_PI);
    readfile(FILE_PI, content.size.h / wmresize->lineheight);
    file_close(FILE_PI);

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

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    file_open(FILE_L0);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_FILE:
            onfile(header);

            break;

        case EVENT_WMKEYPRESS:
            onwmkeypress(header);

            break;

        case EVENT_WMKEYRELEASE:
            onwmkeyrelease(header);

            break;

        case EVENT_WMRESIZE:
            onwmresize(header);

            break;

        case EVENT_WMSHOW:
            onwmshow(header);

            break;

        case EVENT_WMHIDE:
            onwmhide(header);

            break;

        }

        if (ring_count(&output))
        {

            event_sendwmflush(FILE_L0, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, ring_count(&output), outputdata);
            ring_reset(&output);

        }

    }

    file_close(FILE_L0);

}

