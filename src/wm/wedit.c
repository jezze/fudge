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

static void onstop(struct channel *channel, void *mdata, unsigned int msize)
{

}

static void onwmconfigure(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmconfigure *wmconfigure = mdata;

    ring_reset(&input1);
    ring_reset(&input2);
    box_setsize(&content.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h - (wmconfigure->lineheight + 2 * wmconfigure->padding));
    box_resize(&content.size, wmconfigure->padding);
    box_setsize(&status.size, wmconfigure->x, wmconfigure->y + wmconfigure->h - (wmconfigure->lineheight + 2 * wmconfigure->padding), wmconfigure->w, (wmconfigure->lineheight + 2 * wmconfigure->padding));
    box_resize(&status.size, wmconfigure->padding);
    file_open(FILE_P0);
    readfile(FILE_P0, content.size.h / wmconfigure->lineheight);
    file_close(FILE_P0);

}

static void onwmkeypress(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;
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

    updatecontent(&channel->i);
    updatestatus(&channel->i);

}

static void onwmkeyrelease(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmkeyrelease *wmkeyrelease = mdata;

    keymod = keymap_modkey(wmkeyrelease->scancode, keymod);

}

static void onwmshow(struct channel *channel, void *mdata, unsigned int msize)
{

    updatecontent(&channel->i);
    updatestatus(&channel->i);

}

static void onwmhide(struct channel *channel, void *mdata, unsigned int msize)
{

    removecontent(&channel->i);
    removestatus(&channel->i);

}

static void onany(struct channel *channel, void *mdata, unsigned int msize)
{

    if (ring_count(&output))
    {

        struct {struct event_header header; char data[FUDGE_BSIZE];} message;

        event_create(&message.header, EVENT_DATA, ring_count(&output));
        memory_write(message.data, FUDGE_BSIZE, outputdata, ring_count(&output), 0);
        ring_reset(&output);
        file_writeall(FILE_G0, &message, message.header.length);

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_ANY, onany);
    channel_setsignal(&channel, EVENT_STOP, onstop);
    channel_setsignal(&channel, EVENT_WMCONFIGURE, onwmconfigure);
    channel_setsignal(&channel, EVENT_WMKEYPRESS, onwmkeypress);
    channel_setsignal(&channel, EVENT_WMKEYRELEASE, onwmkeyrelease);
    channel_setsignal(&channel, EVENT_WMSHOW, onwmshow);
    channel_setsignal(&channel, EVENT_WMHIDE, onwmhide);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);
    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    widget_inittextbox(&content);
    widget_inittext(&status, WIDGET_TEXTTYPE_HIGHLIGHT);
    channel_request(&channel, EVENT_WMMAP);
    file_writeall(FILE_G0, &channel.o, channel.o.header.length);
    channel_listen(&channel);
    channel_request(&channel, EVENT_WMUNMAP);
    file_writeall(FILE_G0, &channel.o, channel.o.header.length);
    file_close(FILE_G0);

}

