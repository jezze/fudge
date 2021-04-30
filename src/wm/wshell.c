#include <fudge.h>
#include <abi.h>
#include <widget.h>

static struct widget_textbox content;
static char outputdata[BUFFER_SIZE];
static struct ring output;
static char inputdata1[BUFFER_SIZE];
static struct ring input1;
static char inputdata2[BUFFER_SIZE];
static struct ring input2;
static char textdata[BUFFER_SIZE];
static struct ring text;

static void print(void *buffer, unsigned int count)
{

    char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ring_avail(&text))
            ring_skip(&text, ring_find(&text, '\n') + 1);

        if (b[i] != '\r')
            ring_write(&text, &b[i], 1);

    }

}

static void printprompt(void)
{

    print("$ ", 2);

}

static void flush(void)
{

    struct message_data data;

    file_notify(FILE_G0, EVENT_WMRENDERDATA, ring_read(&output, data.buffer, MESSAGE_SIZE), &data);

}

static void updatecontent(void)
{

    content.length = ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_BOTTOM, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length, &content.size);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);
    flush();

}

static void moveleft(unsigned int steps)
{

    char buffer[BUFFER_SIZE];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[BUFFER_SIZE];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void check(struct channel *channel, void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_wmkeypress *wmkeypress = mdata;

    job_send(channel, jobs, njobs, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

    switch (wmkeypress->scancode)
    {

    case 0x10:
        if (wmkeypress->keymod & KEYMOD_SHIFT)
            job_kill(channel, jobs, njobs);
        else
            job_send(channel, jobs, njobs, EVENT_TERM, 0, 0);

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

        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_sendredirectback(channel, id, EVENT_CLOSE);
        channel_send(channel, id, EVENT_DATA, count, buffer);
        channel_send(channel, id, EVENT_MAIN, 0, 0);

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
            {

                unsigned int n = job_parse(jobs, 32, data.buffer, message_datasize(&header));

                njobs = job_spawn(channel, jobs, n);

            }

        }

        job_pipe(channel, jobs, njobs);

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

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);

    print(buffer, count);
    updatecontent();

    if (count >= 2)
        runcommand(channel, count, buffer);

    printprompt();
    updatecontent();

}

static void complete(struct channel *channel, struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);
    unsigned int id = file_spawn("/bin/complete");

    if (id)
    {

        struct message_header header;
        struct message_data data;
        unsigned int offset = 0;

        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_sendredirectback(channel, id, EVENT_CLOSE);
        channel_send(channel, id, EVENT_DATA, count, buffer);
        channel_send(channel, id, EVENT_MAIN, 0, 0);

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
                offset += buffer_write(buffer, BUFFER_SIZE, data.buffer, message_datasize(&header), offset);

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

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        file_duplicate(FILE_PW, FILE_L0);
        file_duplicate(FILE_CW, FILE_L0);

    }

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
    channel_close(channel);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    printprompt();
    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

    while (channel_poll(channel, &header, &data))
        channel_dispatch(channel, &header, &data);

}

void init(struct channel *channel)
{

    ring_init(&output, BUFFER_SIZE, outputdata);
    ring_init(&input1, BUFFER_SIZE, inputdata1);
    ring_init(&input2, BUFFER_SIZE, inputdata2);
    ring_init(&text, BUFFER_SIZE, textdata);
    widget_inittextbox(&content);

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_TERM, onterm);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_PATH, onpath);
    channel_setcallback(channel, EVENT_WMKEYPRESS, onwmkeypress);
    channel_setcallback(channel, EVENT_WMSHOW, onwmshow);

}

