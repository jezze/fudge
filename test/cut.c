#include <fudge.h>
#include <abi.h>

/* the stage is killed once the sink has had REPORT pages; every page must come back to the pool */
#define BYTES                           (PAGE_SIZE * PAGE_COUNT * 4 + 123)
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

static unsigned int readfree(void)
{

    char data[FREESIZE];
    unsigned int count = system_feed(0, 0, data, FREESIZE, "mq -query . -format {free} sysinfo:pages.mq");

    return cstring_read_value(data, buffer_findbyte(data, count, '\n'), 10);

}

static void onmain(struct message *message)
{

    unsigned int targets[STAGES];
    struct message_pipe pipe;
    struct message current;
    unsigned int bytes = BYTES;
    unsigned int delay = DELAY;
    unsigned int report = REPORT;
    unsigned int count = 0;
    unsigned int killed = 0;
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

    channel_send_fmt(1, targets[0], MESSAGE_TYPE_OPTION, "target\\0%u\\0bytes\\0%u\\0", &targets[1], &bytes);
    channel_send_fmt(1, targets[1], MESSAGE_TYPE_OPTION, "target\\0%u\\0", &targets[2]);
    channel_send_fmt(1, targets[2], MESSAGE_TYPE_OPTION, "bytes\\0%u\\0delay\\0%u\\0report\\0%u\\0", &bytes, &delay, &report);
    message_initpipe(&pipe);

    for (i = 0; i < STAGES; i++)
    {

        channel_send(1, targets[i], MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
        channel_send(1, targets[i], MESSAGE_TYPE_TERM, 0, 0);

    }

    while (done < STAGES && channel_pick(1, &current))
    {

        struct message_status *status;

        for (i = 0; i < STAGES && targets[i] != current.source; i++);

        if (i == STAGES)
            continue;

        switch (current.type)
        {

        /* after the kill, failing and aborting is what the stages are expected to do */
        case MESSAGE_TYPE_ERROR:
            if (!killed)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%w", current.data, &current.length);

            break;

        case MESSAGE_TYPE_DATA:
            if (i == 2 && !killed++)
                call_kill(targets[1]);

            break;

        case MESSAGE_TYPE_STATUS:
            status = message_validate(MESSAGE_TYPE_STATUS, current.data, current.length);

            if (i == 1 && (!status || status->code != MESSAGE_STATUS_ABORTED))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "stage was not aborted\n");

            /* the sink waits for pages that will never come */
            if (i == 1)
                call_kill(targets[2]);

            done++;

            break;

        }

    }

    for (i = 0; i < TRIES && (count = readfree()) != PAGE_COUNT; i++);

    if (count != PAGE_COUNT)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "only %u pages came back\n", &count);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

