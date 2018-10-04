#include <abi.h>
#include <fudge.h>
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

static void oninit(struct event_header *header, void *message)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    widget_inittextbox(&content);
    widget_inittext(&status, WIDGET_TEXTTYPE_HIGHLIGHT);
    event_request(message, header, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_send(message);

}

static void onkill(struct event_header *header, void *message)
{

    event_request(message, header, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_send(message);
    event_reply(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

}

static void onfile(struct event_header *header, void *message)
{

}

static void onwmkeypress(struct event_header *header, void *message)
{

    struct event_wmkeypress *wmkeypress = event_getdata(header);
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, wmkeypress->scancode, keymod);

    keymod = keymap_modkey(wmkeypress->scancode, keymod);

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);

        break;

    case 0x47:
        moveleft(ring_findreverse(&input1, '\n'));

        break;

    case 0x48:
        moveup();

        break;

    case 0x4B:
        moveleft(1);

        break;

    case 0x4D:
        moveright(1);

        break;

    case 0x4F:
        moveright(ring_find(&input2, '\n'));

        break;

    case 0x50:
        movedown();

        break;

    default:
        ring_write(&input1, keycode->value, keycode->length);

        break;

    }

    updatecontent(header);
    updatestatus(header);

}

static void onwmkeyrelease(struct event_header *header, void *message)
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

static void onwmresize(struct event_header *header, void *message)
{

    struct event_wmresize *wmresize = event_getdata(header);

    ring_reset(&input1);
    ring_reset(&input2);
    box_setsize(&content.size, wmresize->x, wmresize->y, wmresize->w, wmresize->h - (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&content.size, wmresize->padding);
    box_setsize(&status.size, wmresize->x, wmresize->y + wmresize->h - (wmresize->lineheight + 2 * wmresize->padding), wmresize->w, (wmresize->lineheight + 2 * wmresize->padding));
    box_resize(&status.size, wmresize->padding);
    file_open(FILE_P0);
    readfile(FILE_P0, content.size.h / wmresize->lineheight);
    file_close(FILE_P0);

}

static void onwmshow(struct event_header *header, void *message)
{

    updatecontent(header);
    updatestatus(header);

}

static void onwmhide(struct event_header *header, void *message)
{

    removecontent(header);
    removestatus(header);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        char message[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header, message);

            break;

        case EVENT_KILL:
            onkill(header, message);

            break;

        case EVENT_FILE:
            onfile(header, message);

            break;

        case EVENT_WMKEYPRESS:
            onwmkeypress(header, message);

            break;

        case EVENT_WMKEYRELEASE:
            onwmkeyrelease(header, message);

            break;

        case EVENT_WMRESIZE:
            onwmresize(header, message);

            break;

        case EVENT_WMSHOW:
            onwmshow(header, message);

            break;

        case EVENT_WMHIDE:
            onwmhide(header, message);

            break;

        }

        if (ring_count(&output))
        {

            struct event_header *flush = event_request(message, header, EVENT_WMFLUSH, EVENT_ADDR_BROADCAST);

            event_addwmflush(flush, ring_count(&output), outputdata);
            event_send(message);
            ring_reset(&output);

        }

    }

    event_close();

}

