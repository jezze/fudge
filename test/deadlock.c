#include <fudge.h>
#include <abi.h>

/* far more mail than both sides can hold, sent from channel 1 without reading the echoes: each ends up waiting to send to the other, and one send must fail rather than both waiting forever */
#define WINDOW                          (MESSAGE_SLOTS * 4)

static unsigned int start(void)
{

    unsigned int target = fs_spawn(1, "initrd:test/peer");

    if (target)
    {

        struct message_pipe pipe;

        message_initpipe(&pipe);
        channel_send(1, target, MESSAGE_TYPE_MAIN, sizeof (struct message_pipe), &pipe);

    }

    return target;

}

static void onmain(struct message *message)
{

    unsigned int peer = start();
    unsigned int failed = 0;
    unsigned int echoes = 0;
    unsigned int i;
    struct message current;

    if (!peer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer spawn failed\n");

        return;

    }

    for (i = 0; i < WINDOW; i++)
    {

        if (!channel_send(1, peer, MESSAGE_TYPE_INFO, sizeof (unsigned int), &i))
            failed++;

    }

    channel_send(1, peer, MESSAGE_TYPE_TERM, 0, 0);

    while (channel_pick(1, &current))
    {

        if (current.source != peer)
            continue;

        if (current.type == MESSAGE_TYPE_INFO)
            echoes++;

        if (current.type == MESSAGE_TYPE_STATUS)
            break;

    }

    if (!failed && echoes == WINDOW)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "all %u went through, so the test did not fill both sides\n", &echoes);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

