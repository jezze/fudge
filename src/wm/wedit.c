#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>

static struct widget_textbox content;
static struct widget_text status;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static unsigned int rendertarget;

static void updatecontent(struct event_header *iheader)
{

    content.length = ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, iheader->target, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void updatestatus(struct event_header *iheader)
{

    status.length = 18;

    widget_update(&output, &status, WIDGET_Z_MIDDLE, iheader->target, WIDGET_TYPE_TEXT, sizeof (struct widget_text) + status.length, status.size.x, status.size.y, status.size.w, status.size.h);
    ring_write(&output, &status, sizeof (struct widget_text));
    ring_write(&output, "^S: Save, ^Q: Quit", 18);

}

static void removecontent(struct event_header *iheader)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, iheader->target);

}

static void removestatus(struct event_header *iheader)
{

    widget_remove(&output, &status, WIDGET_Z_MIDDLE, iheader->target);

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

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    return 0;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    widget_inittextbox(&content);
    widget_inittext(&status, WIDGET_TEXTTYPE_HIGHLIGHT);
    event_requestwmmap(oheader, iheader, EVENT_BROADCAST, 0);
    event_send(oheader);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_requestwmunmap(oheader, iheader, EVENT_BROADCAST, 0);
    event_send(oheader);

    return 1;

}

static unsigned int onwmconfigure(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmconfigure *wmconfigure = event_getdata(iheader);

    rendertarget = wmconfigure->rendertarget;

    ring_reset(&input1);
    ring_reset(&input2);
    box_setsize(&content.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h - (wmconfigure->lineheight + 2 * wmconfigure->padding));
    box_resize(&content.size, wmconfigure->padding);
    box_setsize(&status.size, wmconfigure->x, wmconfigure->y + wmconfigure->h - (wmconfigure->lineheight + 2 * wmconfigure->padding), wmconfigure->w, (wmconfigure->lineheight + 2 * wmconfigure->padding));
    box_resize(&status.size, wmconfigure->padding);
    file_open(FILE_P0);
    readfile(FILE_P0, content.size.h / wmconfigure->lineheight);
    file_close(FILE_P0);

    return 0;

}

static unsigned int onwmkeypress(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmkeypress *wmkeypress = event_getdata(iheader);
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

    updatecontent(iheader);
    updatestatus(iheader);

    return 0;

}

static unsigned int onwmkeyrelease(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmkeyrelease *wmkeyrelease = event_getdata(iheader);

    keymod = keymap_modkey(wmkeyrelease->scancode, keymod);

    return 0;

}

static unsigned int onwmshow(struct event_header *iheader, struct event_header *oheader)
{

    updatecontent(iheader);
    updatestatus(iheader);

    return 0;

}

static unsigned int onwmhide(struct event_header *iheader, struct event_header *oheader)
{

    removecontent(iheader);
    removestatus(iheader);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    char ibuffer[FUDGE_BSIZE];
    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = event_init(ibuffer, obuffer);

    event_open();

    while (!status)
    {

        struct event_header *iheader = event_read(ibuffer);

        switch (iheader->type)
        {

        case EVENT_FILE:
            status = onfile(iheader, oheader);

            break;

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        case EVENT_WMCONFIGURE:
            status = onwmconfigure(iheader, oheader);

            break;

        case EVENT_WMKEYPRESS:
            status = onwmkeypress(iheader, oheader);

            break;

        case EVENT_WMKEYRELEASE:
            status = onwmkeyrelease(iheader, oheader);

            break;

        case EVENT_WMSHOW:
            status = onwmshow(iheader, oheader);

            break;

        case EVENT_WMHIDE:
            status = onwmhide(iheader, oheader);

            break;

        }

        if (ring_count(&output))
        {

            event_requestdata(oheader, iheader, rendertarget, 0);
            event_appenddata(oheader, ring_count(&output), outputdata);
            event_send(oheader);
            ring_reset(&output);

        }

    }

    event_close();

}

