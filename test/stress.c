#include <fudge.h>
#include <abi.h>

/* real pipelines run side by side; a freeze shows as a hang, pressure on a mailbox as the kernel's retry log */
#define HELPERS                         4

static void setoption(unsigned int target, char *key, unsigned int value)
{

    char text[16];

    cstring_write_fmt(text, 16, 0, "%u\\0", &value);
    channel_sendoption(1, target, key, text);

}

static void onmain(struct message *message)
{

    unsigned int helpers[HELPERS];
    unsigned int started;
    unsigned int done = 0;
    unsigned int i;
    struct message *current;

    /* spawning waits on channel 1 and drops other mail there, so all are spawned before any starts */
    for (started = 0; started < HELPERS; started++)
    {

        helpers[started] = fs_spawn(1, "initrd:test/repeat");

        if (!helpers[started])
            break;

    }

    for (i = 0; i < started; i++)
    {

        struct message_pipe pipe;

        message_initpipe(&pipe);
        setoption(helpers[i], "command", i);
        channel_send(1, helpers[i], MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
        channel_send(1, helpers[i], MESSAGE_TYPE_TERM, 0, 0);

    }

    if (started < HELPERS)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "only %u helpers started\n", &started);

    while (done < started && (current = channel_pick(1)))
    {

        for (i = 0; i < started && helpers[i] != current->source; i++);

        if (i == started)
            continue;

        if (current->type == MESSAGE_TYPE_ERROR)
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%w", current->data, &current->length);

        if (current->type == MESSAGE_TYPE_STATUS)
            done++;

    }

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

