#include <fudge.h>
#include <abi.h>

static char inputbuffer[FUDGE_BSIZE];
static struct ring input;

static void printprompt(void)
{

    file_writeall(FILE_G1, "$ ", 2);

}

static void check(struct channel *channel, void *mdata, struct job *jobs, unsigned int njobs)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case 'q':
        job_term(channel, jobs, njobs);

        break;

    }

}

static void runcommand(struct channel *channel, unsigned int count, void *buffer)
{

    unsigned int id = job_simple(channel, "/bin/slang", count, buffer);

    if (id)
    {

        struct message_header header;
        struct message_data data;
        struct job jobs[32];
        unsigned int njobs = 0;
        unsigned int nids;

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

        nids = job_run(channel, jobs, njobs);

        while (nids && channel_poll(channel, &header, &data))
        {

            switch (header.event)
            {

            case EVENT_CLOSE:
                nids = job_close(channel, header.source, jobs, njobs);

                break;

            case EVENT_CONSOLEDATA:
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

    struct message_data data;
    unsigned int count = ring_read(ring, data.buffer, FUDGE_MSIZE);
    unsigned int id;

    id = job_simple(channel, "/bin/complete", count, data.buffer);

    if (id)
    {

        struct message_header header;

        while (channel_pollsource(channel, id, &header, &data))
        {

            if (header.event == EVENT_CLOSE)
                break;

            if (header.event == EVENT_DATA)
                file_writeall(FILE_G1, data.buffer, message_datasize(&header));

        }

    }

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        complete(channel, &input);
        printprompt();

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

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        file_writeall(FILE_G1, &consoledata->data, 1);

        break;

    }

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_L0, mdata))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "event"))
        return;

    if (!file_walk(FILE_G1, FILE_L0, "transmit"))
        return;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_open(FILE_G0);
    file_open(FILE_G1);
    printprompt();

    while (channel_poll(channel, &header, &data))
        channel_dispatch(channel, &header, &data);

    file_close(FILE_G1);
    file_close(FILE_G0);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_writeall(FILE_G1, mdata, msize);

}

void init(struct channel *channel)
{

    ring_init(&input, FUDGE_BSIZE, inputbuffer);

    if (!file_walk2(FILE_G0, "/system/console/if:0/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/console/if:0/transmit"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

