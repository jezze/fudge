#include <fudge.h>
#include <abi.h>

/* the stage is killed once the sink has had REPORT pages; every page must come back to the pool */
#define BYTES                           (PAGE_SIZE * PAGE_TASKLIMIT * 32 + 123)
#define DELAY                           20000
#define STAGES                          3
#define REPORT                          8
#define TRIES                           20
#define FREESIZE                        32

static char *names[STAGES] = {
    "source",
    "stage",
    "sink"
};

static void setoption(unsigned int target, char *key, unsigned int value)
{

    char text[16];

    cstring_write_fmt(text, 16, 0, "%u\\0", &value);
    channel_sendoption(1, target, key, text);

}

static unsigned int readfree(void)
{

    char data[FREESIZE];
    unsigned int count = system_feed(0, 0, data, FREESIZE, "mq -query . -format {free} sysinfo:pages.mq");

    return cstring_read_value(data, buffer_findbyte(data, count, '\n'), 10);

}

/* wired as sh would wire it: a stage's input is the one before it, and it passes TERM on to the next when it ends */
static void start(unsigned int target, unsigned int prev, unsigned int next)
{

    struct message_pipe pipe;

    message_initpipe(&pipe);

    pipe.prev = prev;

    if (next)
    {

        pipe.nterms = 1;
        pipe.terms[0].event = MESSAGE_TYPE_TERM;
        pipe.terms[0].to = MESSAGE_TYPE_TERM;
        pipe.terms[0].target = next;

    }

    channel_send(1, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);

    if (!prev)
        channel_send(1, target, MESSAGE_TYPE_TERM, 0, 0);

}

static void onmain(struct message *message)
{

    unsigned int targets[STAGES];
    struct message *current;
    unsigned int bytes = BYTES;
    unsigned int delay = DELAY;
    unsigned int report = REPORT;
    unsigned int count = 0;
    unsigned int killed = 0;
    unsigned int before = readfree();
    unsigned int done = 0;
    unsigned int i;

    /* spawning waits on channel 1 and drops other mail there, so all are spawned before any starts */
    targets[0] = fs_spawn(1, "initrd:test/source");
    targets[1] = fs_spawn(1, "initrd:test/stage");
    targets[2] = fs_spawn(1, "initrd:test/sink");

    for (i = 0; i < STAGES; i++)
    {

        if (!targets[i])
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s spawn failed\n", names[i]);

            return;

        }

    }

    setoption(targets[0], "target", targets[1]);
    setoption(targets[0], "bytes", bytes);
    setoption(targets[1], "target", targets[2]);
    setoption(targets[2], "bytes", bytes);
    setoption(targets[2], "delay", delay);
    setoption(targets[2], "report", report);
    start(targets[2], targets[1], 0);
    start(targets[1], targets[0], targets[2]);
    start(targets[0], 0, targets[1]);

    while (done < STAGES && (current = channel_pick(1)))
    {

        struct message_status *status;

        for (i = 0; i < STAGES && targets[i] != current->source; i++);

        if (i == STAGES)
            continue;

        switch (current->type)
        {

        /* after the kill, failing and aborting is what the stages are expected to do */
        case MESSAGE_TYPE_ERROR:
            if (!killed)
                channel_send(0, message->source, MESSAGE_TYPE_ERROR, current->length, current->data);

            break;

        case MESSAGE_TYPE_DATA:
            if (i == 2 && !killed++)
                call_kill(targets[1]);

            break;

        case MESSAGE_TYPE_STATUS:
            status = message_validate(MESSAGE_TYPE_STATUS, current->data, current->length);

            if (i == 1 && (!status || status->code != MESSAGE_STATUS_ABORTED))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "stage was not aborted\n");

            /* the sink waits for pages that will never come */
            if (i == 1)
                call_kill(targets[2]);

            done++;

            break;

        }

    }

    for (i = 0; i < TRIES && (count = readfree()) != before; i++);

    if (count != before)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "only %u of %u pages came back\n", &count, &before);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

