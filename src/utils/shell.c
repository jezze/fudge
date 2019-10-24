#include <fudge.h>
#include <abi.h>

static char inputbuffer[FUDGE_BSIZE];
static struct ring input;
static unsigned int idcomplete;
static unsigned int idslang;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static void startchild(struct channel *channel, unsigned int id)
{

    channel_request(channel, EVENT_OPEN);
    channel_place(channel, id);

}

static void stopchild(struct channel *channel, unsigned int id)
{

    channel_request(channel, EVENT_CLOSE);
    channel_place(channel, id);

}

static unsigned int interpretbuiltin(unsigned int count, char *data)
{

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

static void interpret(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_BSIZE);

    if (count < 2)
        return;

    if (interpretbuiltin(count, data))
        return;

    channel_request(channel, EVENT_DATA);
    channel_append(channel, count, data);
    channel_place(channel, idslang);

}

static void complete(struct channel *channel, struct ring *ring)
{

    char data[FUDGE_BSIZE];
    unsigned int count = ring_read(ring, data, FUDGE_BSIZE);

    channel_request(channel, EVENT_DATA);
    channel_append(channel, count, data);
    channel_place(channel, idcomplete);

}

static void onconsoledata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(channel, &input);

        break;

    case '\b':
    case 0x7F:
        if (!ring_skipreverse(&input, 1))
            break;

        file_writeall(FILE_G1, "\b \b", 3);

        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        file_writeall(FILE_G1, &consoledata->data, 1);
        ring_write(&input, &consoledata->data, 1);
        interpret(channel, &input);
        printprompt();

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    if (channel->i.source == idcomplete)
    {

        file_writeall(FILE_G1, mdata, msize);
        printprompt();

    }

    else if (channel->i.source == idslang)
    {

        struct job jobs[32];

        job_interpret(jobs, 32, channel, mdata, msize);

    }

    else
    {

        file_writeall(FILE_G1, "\b\b  \b\b", 6);
        file_writeall(FILE_G1, mdata, msize);
        printprompt();

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    ring_init(&input, FUDGE_BSIZE, inputbuffer);

    if (!file_walk(FILE_G0, FILE_P0, "event"))
        return;

    if (!file_walk(FILE_G1, FILE_P0, "odata"))
        return;

    if (!file_walk2(FILE_CP, "/bin/complete"))
        return;

    idcomplete = call_spawn();

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    idslang = call_spawn();

    startchild(&channel, idcomplete);
    startchild(&channel, idslang);
    file_open(FILE_G0);
    file_open(FILE_G1);
    printprompt();
    channel_listen(&channel);
    file_close(FILE_G1);
    file_close(FILE_G0);
    stopchild(&channel, idcomplete);
    stopchild(&channel, idslang);

}

