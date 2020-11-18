#include <fudge.h>
#include <abi.h>
#include <widget.h>

static struct widget_textbox content;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static char inputdata1[FUDGE_BSIZE];
static struct ring input1;
static char inputdata2[FUDGE_BSIZE];
static struct ring input2;
static char textdata[FUDGE_BSIZE];
static struct ring text;

static void print(void *buffer, unsigned int count)
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

static void printprompt(void)
{

    print("$ ", 2);

}

static void updatecontent(void)
{

    struct message_data data;
    unsigned int count;

    content.length = ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_BOTTOM, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, &content.size);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);

    while ((count = ring_read(&output, data.buffer, FUDGE_MSIZE)))
        file_notify(FILE_G0, EVENT_DATA, count, &data);

    ring_reset(&output);

}

static void copyring(struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int head = ring->head;
    unsigned int tail = ring->tail;

    while ((count = ring_read(ring, buffer, FUDGE_BSIZE)))
        print(buffer, count);

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

static void check(struct channel *channel, void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->scancode)
    {

    case 0x10:
        job_term(channel, jobs, njobs);

        break;

    }

}

static void runcommand(struct channel *channel, unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int tasks;

        job_replyback(channel, id, EVENT_DATA);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_DATA, count, buffer);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
            {

                struct job_status status;

                status.start = data.buffer;
                status.end = status.start + message_datasize(&header);

                while (status.start < status.end)
                    njobs = job_parse(&status, jobs, 32);

            }

        }

        tasks = job_run(channel, jobs, njobs);

        while (tasks && channel_poll(channel, &header, &data))
        {

            switch (header.event)
            {

            case EVENT_CLOSE:
                tasks = job_close(channel, header.source, jobs, njobs);

                break;

            case EVENT_WMKEYPRESS:
                check(channel, data.buffer, jobs, njobs);

                break;

            default:
                channel_dispatch(channel, &header, &data);

                break;

            }

        }

    }

}

static void interpret(struct channel *channel, struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, buffer, FUDGE_BSIZE);

    if (count >= 2)
    {

        if (memory_match(buffer, "cd ", 3))
        {

            buffer[count - 1] = '\0';

            if (file_walk2(FILE_L0, buffer + 3))
            {

                file_duplicate(FILE_PW, FILE_L0);
                file_duplicate(FILE_CW, FILE_L0);

            }

        }

        else
        {

            runcommand(channel, count, buffer);

        }

    }

    printprompt();

}

static void complete(struct channel *channel, struct ring *ring)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, buffer, FUDGE_BSIZE);
    unsigned int id = file_spawn("/bin/complete");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        unsigned int offset = 0;

        job_replyback(channel, id, EVENT_DATA);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_DATA, count, buffer);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
                offset += memory_write(buffer, FUDGE_BSIZE, data.buffer, message_datasize(&header), offset);

        }

        if (offset)
        {

            unsigned int nlines = 0;
            unsigned int i;

            for (i = 0; i < offset; i++)
            {

                if (buffer[i] == '\n')
                    nlines++;

            }

            if (nlines > 1)
            {

                print("\n", 1);
                print(buffer, offset);
                printprompt();

            }

            else
            {

                print(buffer, offset - 1);

            }

        }

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    print(mdata, msize);
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
        {

            ring_reset(&text);
            printprompt();

        }

        else
        {

            ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);

        }

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

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close(channel, source);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

    while (channel_poll(channel, &header, &data))
        channel_dispatch(channel, &header, &data);

}

void init(struct channel *channel)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    ring_init(&input1, FUDGE_BSIZE, inputdata1);
    ring_init(&input2, FUDGE_BSIZE, inputdata2);
    ring_init(&text, FUDGE_BSIZE, textdata);
    printprompt();
    widget_inittextbox(&content);

    if (!file_walk2(FILE_G0, "/system/wserver"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_TERM, onterm);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_WMKEYPRESS, onwmkeypress);
    channel_setcallback(channel, EVENT_WMSHOW, onwmshow);

}

