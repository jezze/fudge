#include <fudge.h>
#include <abi.h>

/* many times what a task may have out, so every page it takes goes round the pipe again and again */
#define BYTES                           (PAGE_SIZE * PAGE_TASKLIMIT * 32 + 123)
#define DELAY                           20000
#define STAGES                          3

static char *names[STAGES] = {
    "source",
    "stage",
    "sink"
};

static void onmain(struct message *message)
{

    unsigned int targets[STAGES];
    struct message_pipe pipe;
    struct message current;
    unsigned int bytes = BYTES;
    unsigned int delay = DELAY;
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
    channel_send_fmt(1, targets[2], MESSAGE_TYPE_OPTION, "bytes\\0%u\\0delay\\0%u\\0", &bytes, &delay);
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

        case MESSAGE_TYPE_ERROR:
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%w", current.data, &current.length);

            break;

        case MESSAGE_TYPE_STATUS:
            status = message_validate(MESSAGE_TYPE_STATUS, current.data, current.length);

            if (!status || status->code != MESSAGE_STATUS_OK)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%s did not exit cleanly\n", names[i]);

            done++;

            break;

        }

    }

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

