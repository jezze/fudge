#include <fudge.h>
#include <abi.h>

#define ROUNDS                          1024

static char sent[MESSAGE_SLOTSIZE];
static char received[MESSAGE_SLOTSIZE];

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

static unsigned int finish(unsigned int target)
{

    struct message message;

    channel_send(1, target, MESSAGE_TYPE_TERM, 0, 0);

    return channel_poll(1, target, MESSAGE_TYPE_STATUS, &message);

}

static void onmain(struct message *message)
{

    unsigned int peer = start();
    unsigned int i;

    if (!peer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer spawn failed\n");

        return;

    }

    for (i = 0; i < ROUNDS; i++)
    {

        unsigned int count = 1 + (i * 97) % MESSAGE_SLOTSIZE;
        unsigned int j;

        for (j = 0; j < count; j++)
            sent[j] = i + j;

        if (!channel_send(1, peer, MESSAGE_TYPE_DATA, count, sent))
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "round %u send failed\n", &i);

            break;

        }

        if (channel_wait(1, peer, MESSAGE_TYPE_DATA, MESSAGE_SLOTSIZE, received) != count || !buffer_match(sent, received, count))
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "round %u echo differs\n", &i);

            break;

        }

    }

    if (!finish(peer))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer gave no status\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

