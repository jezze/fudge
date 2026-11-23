#include <fudge.h>
#include <abi.h>

/* more than the kernel's task pool holds, so retired tasks must be reused */
#define SEQUENTIAL                      512
#define CONCURRENT                      16

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

    unsigned int targets[CONCURRENT];
    unsigned int started;
    unsigned int i;

    for (i = 0; i < SEQUENTIAL; i++)
    {

        unsigned int target = start();

        if (!target || !finish(target))
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sequential spawn %u failed\n", &i);

            return;

        }

    }

    for (started = 0; started < CONCURRENT; started++)
    {

        targets[started] = start();

        if (!targets[started])
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "concurrent spawn %u failed\n", &started);

            break;

        }

    }

    for (i = 0; i < started; i++)
    {

        if (!finish(targets[i]))
            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "concurrent task %u gave no status\n", &i);

    }

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

