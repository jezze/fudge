#include <fudge.h>
#include <abi.h>

/* control mail, so the mailbox is what is tested: the same window as window0 but from channel 1, where echoes pile up while this task is still sending */
#define WINDOW                          MESSAGE_SLOTS

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

    for (i = 0; i < WINDOW; i++)
        channel_send(1, peer, MESSAGE_TYPE_INFO, sizeof (unsigned int), &i);

    for (i = 0; i < WINDOW; i++)
    {

        unsigned int value;

        if (channel_wait(1, peer, MESSAGE_TYPE_INFO, sizeof (unsigned int), &value) != sizeof (unsigned int) || value != i)
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "echo %u wrong\n", &i);

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

