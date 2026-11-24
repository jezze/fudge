#include <fudge.h>
#include <abi.h>

#define SENDERS                         8
#define COUNT                           256

static void setoption(unsigned int target, char *key, unsigned int value)
{

    char text[16];

    cstring_write_fmt(text, 16, 0, "%u\\0", &value);
    channel_sendoption(1, target, key, text);

}

static void start(unsigned int target)
{

    struct message_pipe pipe;
    unsigned int count = COUNT;

    message_initpipe(&pipe);
    setoption(target, "count", count);
    channel_send(1, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);
    channel_send(1, target, MESSAGE_TYPE_TERM, 0, 0);

}

static unsigned int find(unsigned int *targets, unsigned int source)
{

    unsigned int i;

    for (i = 0; i < SENDERS; i++)
    {

        if (targets[i] == source)
            return i;

    }

    return SENDERS;

}

static void onmain(struct message *message)
{

    unsigned int targets[SENDERS];
    unsigned int next[SENDERS];
    unsigned int started;
    unsigned int done = 0;
    unsigned int i;
    struct message current;

    for (started = 0; started < SENDERS; started++)
    {

        next[started] = 0;
        targets[started] = fs_spawn(1, "initrd:test/sender");

        if (!targets[started])
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sender %u spawn failed\n", &started);

            break;

        }

    }

    /* spawning waits on channel 1 and drops other mail there, so no sender starts before all exist */
    for (i = 0; i < started; i++)
        start(targets[i]);

    while (done < started && channel_pick(1, &current))
    {

        struct message_status *status;
        unsigned int value;

        i = find(targets, current.source);

        if (i == SENDERS)
            continue;

        switch (current.type)
        {

        case MESSAGE_TYPE_DATA:
            if (current.length != sizeof (unsigned int))
                break;

            buffer_copy(&value, current.data, sizeof (unsigned int));

            if (value != next[i])
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sender %u sent %u, expected %u\n", &i, &value, &next[i]);

            next[i] = value + 1;

            break;

        case MESSAGE_TYPE_STATUS:
            status = message_validate(MESSAGE_TYPE_STATUS, current.data, current.length);

            if (!status || status->code != MESSAGE_STATUS_OK)
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sender %u did not exit cleanly\n", &i);

            done++;

            break;

        }

    }

    for (i = 0; i < started; i++)
    {

        if (next[i] != COUNT)
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sender %u: %u of its messages arrived\n", &i, &next[i]);

    }

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

