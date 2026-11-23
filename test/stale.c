#include <fudge.h>
#include <abi.h>

#define STALE                           0xDEAD

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

    unsigned int old = start();
    unsigned int peer;
    unsigned int value = STALE;
    unsigned int i;

    if (!old || !finish(old))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "first peer did not run\n");

        return;

    }

    peer = start();

    if (!peer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer spawn failed\n");

        return;

    }

    if (peer == old)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "new peer got the old peer's inode\n");

    /* the old mailbox may take mail until it is retired, never more than it holds */
    for (i = 0; i <= MESSAGE_SLOTS && channel_send(1, old, MESSAGE_TYPE_DATA, sizeof (unsigned int), &value); i++);

    if (i > MESSAGE_SLOTS)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "exited peer still takes mail\n");

    value = 1;

    channel_send(1, peer, MESSAGE_TYPE_DATA, sizeof (unsigned int), &value);

    if (!channel_wait(1, peer, MESSAGE_TYPE_DATA, sizeof (unsigned int), &value) || value != 1)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "new peer got the stale mail\n");

    if (!finish(peer))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer gave no status\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

