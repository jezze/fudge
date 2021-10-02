#include <fudge.h>
#include <abi.h>
#include <widget.h>

static struct widget_textbox content;
static char outputdata[BUFFER_SIZE];
static struct ring output;
static char inputdata1[128];
static struct ring input1;
static char inputdata2[128];
static struct ring input2;
static char textdata[1024];
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

    char buffer[MESSAGE_SIZE];

    file_notify(FILE_G0, EVENT_WMRENDERDATA, ring_read(&output, buffer, MESSAGE_SIZE), buffer);

}

static void updatecontent(void)
{

    content.length = ring_count(&text) + ring_count(&input1) + ring_count(&input2) + 1;
    content.cursor = ring_count(&text) + ring_count(&input1);

    widget_update(&output, &content, WIDGET_Z_BOTTOM, WIDGET_TYPE_TEXTBOX, sizeof (struct widget_textbox) + content.length);
    ring_write(&output, &content, sizeof (struct widget_textbox));
    ring_copy(&output, &text);
    ring_copy(&output, &input1);
    ring_copy(&output, &input2);
    ring_write(&output, "\n", 1);
    flush();

}

static void moveleft(unsigned int steps)
{

    char buffer[128];

    ring_writereverse(&input2, buffer, ring_readreverse(&input1, buffer, steps));

}

static void moveright(unsigned int steps)
{

    char buffer[128];

    ring_write(&input1, buffer, ring_read(&input2, buffer, steps));

}

static void check(void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->scancode)
    {

    case 0x2E:
        if (wmkeypress->keymod & KEYMOD_CTRL)
            job_send(jobs, njobs, EVENT_TERM, 0, 0);
        else
            job_send(jobs, njobs, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        break;

    default:
        job_send(jobs, njobs, EVENT_CONSOLEDATA, wmkeypress->length, &wmkeypress->unicode);

        break;

    }

}

static void runcommand(unsigned int count, void *buffer)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message message;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int tasks;

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
        {

            unsigned int n = job_parse(jobs, 32, message.data.buffer, count);

            njobs = job_spawn(jobs, n);

        }

        job_pipe(jobs, njobs);

        tasks = job_run(jobs, njobs);

        while (tasks && channel_pick(&message))
        {

            switch (message.header.event)
            {

            case EVENT_CLOSE:
                tasks = job_close(message.header.source, jobs, njobs);

                break;

            case EVENT_WMKEYPRESS:
                check(message.data.buffer, jobs, njobs);

                break;

            default:
                channel_dispatch(&message);

                break;

            }

        }

    }

}

static void interpret(struct ring *ring)
{

    char buffer[128];
    unsigned int count = ring_read(ring, buffer, 128);

    print(buffer, count);
    updatecontent();

    if (count >= 2)
        runcommand(count, buffer);

    printprompt();
    updatecontent();

}

static void complete(struct ring *ring)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = ring_read(ring, buffer, BUFFER_SIZE);
    unsigned int id = file_spawn("/bin/complete");

    if (id)
    {

        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendbufferto(id, EVENT_DATA, count, buffer);
        channel_sendto(id, EVENT_MAIN);

        while (channel_readfrom(id, buffer, BUFFER_SIZE));

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(mdata, msize);
    updatecontent();

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    printprompt();
    file_notify(FILE_G0, EVENT_WMMAP, 0, 0);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_CW, mdata))
        file_duplicate(FILE_CW, FILE_L0);

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    file_notify(FILE_G0, EVENT_WMUNMAP, 0, 0);
    channel_close();

}

static void onwmkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmkeypress *wmkeypress = mdata;

    switch (wmkeypress->scancode)
    {

    case 0x0E:
        ring_skipreverse(&input1, 1);

        break;

    case 0x0F:
        ring_move(&input1, &input2);
        complete(&input1);

        break;

    case 0x1C:
        ring_move(&input1, &input2);
        ring_write(&input1, &wmkeypress->unicode, wmkeypress->length);
        interpret(&input1);

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

static void onwmmousescroll(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_wmmousescroll *wmmousescroll = mdata;

    content.scroll += wmmousescroll->relz;

    updatecontent();

}

static void onwmshow(unsigned int source, void *mdata, unsigned int msize)
{

    updatecontent();

}

void init(void)
{

    ring_init(&output, BUFFER_SIZE, outputdata);
    ring_init(&input1, 128, inputdata1);
    ring_init(&input2, 128, inputdata2);
    ring_init(&text, 1024, textdata);
    widget_inittextbox(&content);

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_WMKEYPRESS, onwmkeypress);
    channel_bind(EVENT_WMMOUSESCROLL, onwmmousescroll);
    channel_bind(EVENT_WMSHOW, onwmshow);

}

