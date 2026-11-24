#include <fudge.h>
#include <abi.h>

/* control mail, so the mailbox is what is tested: sends a window of it from channel 0 before reading any echo; a full channel 0 is relieved while sending */
#define WINDOW                          (MESSAGE_SLOTS * 4)

static unsigned int peer;
static unsigned int received;
static unsigned int disorder;
static unsigned int firstexpected;
static unsigned int firstvalue;

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

static void ondata(struct message *message)
{

    unsigned int value;

    if (message->source != peer || message->length != sizeof (unsigned int))
        return;

    buffer_copy(&value, message->data, sizeof (unsigned int));

    if (value != received && !disorder++)
    {

        firstexpected = received;
        firstvalue = value;

    }

    received++;

}

static void onmain(struct message *message)
{

    unsigned int i;

    peer = start();

    if (!peer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer spawn failed\n");

        return;

    }

    for (i = 0; i < WINDOW; i++)
        channel_send(0, peer, MESSAGE_TYPE_INFO, sizeof (unsigned int), &i);

    while (received < WINDOW && channel_process(0));

    if (received != WINDOW)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u of %u echoes\n", &received, &i);

    if (disorder)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "%u echoes out of order, first was %u where %u was due\n", &disorder, &firstvalue, &firstexpected);

    if (!finish(peer))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer gave no status\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_INFO, ondata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

