#include <fudge.h>
#include <abi.h>

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
    unsigned int value = 1;
    struct message *current;
    struct message_status *status;

    if (!peer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer spawn failed\n");

        return;

    }

    channel_send(1, peer, MESSAGE_TYPE_DATA, sizeof (unsigned int), &value);

    if (!channel_wait(1, peer, MESSAGE_TYPE_DATA, sizeof (unsigned int), &value))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer not alive\n");

        return;

    }

    /* the peer now waits for mail: a first kill makes it abort and tell its owner */
    if (call_kill(peer) != MESSAGE_OK)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "kill failed\n");

    if (!(current = channel_poll(1, peer, MESSAGE_TYPE_STATUS)))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "peer gave no status\n");

        return;

    }

    status = message_validate(MESSAGE_TYPE_STATUS, current->data, current->length);

    if (!status || status->code != MESSAGE_STATUS_ABORTED)
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "status was not an abort\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

