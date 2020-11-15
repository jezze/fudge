#include <fudge.h>
#include <abi.h>
#include <widget.h>

static struct widget_textbox content;
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
static struct job jobs[32];
static unsigned int njobs;

static void updatecontent(void)
{

    content.length = ring_count(&text) + ring_count(&prompt) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&prompt) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_BOTTOM, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, &content.size);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &prompt);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

}

static void copybuffer(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&text))
            ring_skip(&text, ring_find(&text, '\n') + 1);

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

static unsigned int interpretbuiltin(unsigned int count, char *data)
{

    if (count < 2)
        return 1;

    if (memory_match(data, "cd ", 3))
    {

        data[count - 1] = '\0';

        if (file_walk2(FILE_L0, data + 3))
        {

            file_duplicate(FILE_PW, FILE_L0);
            file_duplicate(FILE_CW, FILE_L0);

        }

        return 1;

    }

    return 0;

}

static void check(struct channel *channel, void *mdata)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->scancode)
    {

    case 0x10:
        job_term(channel, jobs, njobs);

        break;

    }

}

static void interpret(struct channel *channel, struct ring *ring)
{

    struct message_data data;
    unsigned int count = ring_read(ring, data.buffer, FUDGE_MSIZE);
    unsigned int id;

    if (interpretbuiltin(count, data.buffer))
        return;

    id = job_simple(channel, "/bin/slang", count, data.buffer);

    if (id)
    {

        struct message_header header;
        unsigned int nids;

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.type == EVENT_CLOSE)
                break;

            if (header.type == EVENT_DATA)
            {

                struct job_status status;

                status.start = data.buffer;
                status.end = status.start + message_datasize(&header);

                while (status.start < status.end)
                    njobs = job_parse(&status, jobs, 32);

            }

        }

        nids = job_run(channel, jobs, njobs);

        while (nids && channel_poll(channel, &header, &data))
        {

            switch (header.type)
            {

            case EVENT_CLOSE:
                nids = job_close(channel, header.source, jobs, njobs);

                break;

            case EVENT_WMKEYPRESS:
                check(channel, data.buffer);

                break;

            default:
                channel_dispatch(channel, &header, &data);

                break;

            }

        }

    }

}

static void complete(struct channel *channel, struct ring *ring)
{

    struct message_data data;
    unsigned int count = ring_read(ring, data.buffer, FUDGE_MSIZE);
    unsigned int id;

    id = job_simple(channel, "/bin/complete", count, data.buffer);

    if (id)
    {

        struct message_header header;

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.type == EVENT_CLOSE)
                break;

            if (header.type == EVENT_DATA)
            {

                if (memory_findbyte(data.buffer, message_datasize(&header), '\n') < message_datasize(&header) - 1)
                {

                    copyring(&prompt);
                    copybuffer("\n", 1);
                    copybuffer(data.buffer, message_datasize(&header));

                }

                else
                {

                    ring_write(&input1, data.buffer, message_datasize(&header) - 1);

                }

            }

        }

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    copybuffer(mdata, msize);
    updatecontent();

}

static void onwmkeypress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;

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
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
        copyring(&prompt);
        copyring(&input1);
        interpret(channel, &input1);

        break;

    case 0x16:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input1);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x25:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&input2);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x26:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            ring_reset(&text);
        else
            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    case 0x47:
        moveleft(ring_count(&input1));

        break;

    case 0x48:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
            content.offset--;

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

    case 0x50:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
            content.offset++;

        break;

    default:
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        break;

    }

    updatecontent();

}

static void onwmshow(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    updatecontent();

}

static void onterm(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;

    message_initheader(&header, EVENT_WMUNMAP, 0);
    file_writeall(FILE_G0, &header, header.length);
    channel_close(channel);

}

static void onany(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (ring_count(&output))
    {

        struct {struct message_header header; struct message_data data;} message;

        message_initheader(&message.header, EVENT_DATA, ring_count(&output));
        message_append(&message.data, 0, ring_count(&output), outputdata);
        file_writeall(FILE_G0, &message, message.header.length);
        ring_reset(&output);

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    message_initheader(&header, EVENT_WMMAP, 0);
    file_open(FILE_G0);
    file_writeall(FILE_G0, &header, header.length);

    while (channel_poll(channel, &header, &data))
        channel_dispatch(channel, &header, &data);

    file_close(FILE_G0);

}

void init(struct channel *channel)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&prompt, FUDGE_BSIZE, promptdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    ring_write(&prompt, "$ ", 2);
    widget_inittextbox(&content);

    if (!file_walk2(FILE_G0, "/system/wclient"))
        return;

    channel_setsignal(channel, EVENT_ANY, onany);
    channel_setsignal(channel, EVENT_MAIN, onmain);
    channel_setsignal(channel, EVENT_TERM, onterm);
    channel_setsignal(channel, EVENT_DATA, ondata);
    channel_setsignal(channel, EVENT_WMKEYPRESS, onwmkeypress);
    channel_setsignal(channel, EVENT_WMSHOW, onwmshow);

}

