#include <fudge.h>
#include <abi.h>
#include <widget.h>

static struct widget_textbox content;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char promptdata[FUDGE_BSIZE];
static struct ring prompt;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static char textdata[FUDGE_BSIZE];
static struct ring text;
static unsigned int totalrows;
static unsigned int visiblerows;

static void updatecontent(unsigned int target)
{

    content.length = ring_count(&text) + ring_count(&prompt) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&prompt) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_MIDDLE, target, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, content.size.x, content.size.y, content.size.w, content.size.h);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &prompt);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void removecontent(unsigned int target)
{

    widget_remove(&output, &content, WIDGET_Z_MIDDLE, target);

}

static void removerows(unsigned int count)
{

    while (count--)
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

        if (!ring_avail(&text))
            removerows(1);

        if (b[i] == '\n')
        {

            totalrows++;

            if (totalrows >= visiblerows)
                removerows(totalrows - visiblerows + 1);

        }

        ring_write(&text, &b[i], 1);

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

static void printnormal(void *buffer, unsigned int count)
{

    copybuffer(buffer, count);

}

static void printcomplete(void *buffer, unsigned int count)
{

    if (memory_findbyte(buffer, count, '\n') < count - 1)
    {

        copyring(&prompt);
        copybuffer("\n", 1);
        copybuffer(buffer, count);

    }

    else
    {

        ring_write(&input1, buffer, count - 1);

    }

}

static unsigned int runcmd(struct channel *channel, char *command, char *data, unsigned int count, unsigned int session)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, command))
        return 0;

    id = call_spawn();

    if (id)
    {

        channel_request2(channel, EVENT_OPEN, session);
        channel_place(channel, id);
        channel_request2(channel, EVENT_DATA, session);
        channel_append(channel, count, data);
        channel_place(channel, id);
        channel_request2(channel, EVENT_CLOSE, session);
        channel_place(channel, id);

    }

    return id;

}

static unsigned int interpretbuiltin(unsigned int count, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        command[count - 1] = '\0';

        if (file_walk2(FILE_L0, command + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static unsigned int interpret(struct channel *channel, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    if (count < 2)
        return 0;

    if (interpretbuiltin(count, command))
        return 0;

    return runcmd(channel, "/bin/slang", command, count, 2);

}

static unsigned int complete(struct channel *channel, struct ring *ring)
{

    char command[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, command, FUDGE_BSIZE);

    return runcmd(channel, "/bin/complete", command, count, 1);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job jobs[32];

    switch (channel->i.session)
    {

    case 0:
        printnormal(mdata, msize);

        break;

    case 1:
        printcomplete(mdata, msize);

        break;

    case 2:
        job_interpret(jobs, 32, channel, mdata, msize, 0);

        break;

    }

    updatecontent(channel->i.target);

}

static void onwmconfigure(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmconfigure *wmconfigure = mdata;

    box_setsize(&content.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);
    box_resize(&content.size, wmconfigure->padding);

    visiblerows = content.size.h / wmconfigure->lineheight;

    if (totalrows >= visiblerows)
        removerows(totalrows - visiblerows + 1);

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

    case 0x0F:
        ring_move(&input1, &input2);
        complete(channel, &input1);

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        ring_write(&input1, keycode->value, keycode->length);
        copyring(&prompt);
        copyring(&input1);
        interpret(channel, &input1);

        break;

    case 0x16:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x25:
        if (keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x26:
        if (keymod & KEYMOD_CTRL)
            removerows(totalrows);
        else
            ring_write(&input1, keycode->value, keycode->length);

        break;

    case 0x47:
        moveleft(ring_count(&input1));

        break;

    case 0x4B:
        moveleft(1);

        break;

    case 0x4D:
        moveright(1);

        break;

    case 0x4F:
        moveright(ring_count(&input2));

        break;

    default:
        ring_write(&input1, keycode->value, keycode->length);

        break;

    }

    updatecontent(channel->i.target);

}

static void onwmkeyrelease(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmkeyrelease *wmkeyrelease = mdata;

    keymod = keymap_modkey(wmkeyrelease->scancode, keymod);

}

static void onwmshow(struct channel *channel, void *mdata, unsigned int msize)
{

    updatecontent(channel->i.target);

}

static void onwmhide(struct channel *channel, void *mdata, unsigned int msize)
{

    removecontent(channel->i.target);

}

static void onwmclose(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_WMUNMAP);

    channel_place(channel, id);
    channel_exit(channel);

}

static void onany(struct channel *channel, void *mdata, unsigned int msize)
{

    if (ring_count(&output))
    {

        struct {struct ipc_header header; char data[FUDGE_BSIZE];} message;

        ipc_create(&message.header, EVENT_DATA, ring_count(&output));
        memory_write(message.data, FUDGE_BSIZE, outputdata, ring_count(&output), 0);
        ring_reset(&output);
        file_writeall(FILE_G0, &message, message.header.length);

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_nosignal(&channel, EVENT_CLOSE);
    channel_setsignal(&channel, EVENT_ANY, onany);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_WMCONFIGURE, onwmconfigure);
    channel_setsignal(&channel, EVENT_WMKEYPRESS, onwmkeypress);
    channel_setsignal(&channel, EVENT_WMKEYRELEASE, onwmkeyrelease);
    channel_setsignal(&channel, EVENT_WMSHOW, onwmshow);
    channel_setsignal(&channel, EVENT_WMHIDE, onwmhide);
    channel_setsignal(&channel, EVENT_WMCLOSE, onwmclose);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    file_open(FILE_G0);
    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&prompt, FUDGE_BSIZE, promptdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    widget_inittextbox(&content);
    ring_write(&prompt, "$ ", 2);
    channel_request(&channel, EVENT_WMMAP);
    file_writeall(FILE_G0, &channel.o, channel.o.length);
    channel_listen(&channel);
    file_close(FILE_G0);

}

