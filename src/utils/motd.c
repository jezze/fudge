#include <fudge.h>
#include <abi.h>

static void ondata(struct message *message)
{

    channel_send(0, message->source, EVENT_DATA, message->length, message->data);

}

static void onmain(struct message *message)
{

    unsigned int target = fs_spawn(1, "initrd:bin/echo");

    if (target)
    {

        channel_route(EVENT_DATA, message->source);
        channel_send(1, target, EVENT_MAIN, 0, 0);
        channel_send_fmt0(1, target, EVENT_PATH, "initrd:data/motd.txt\\0");
        channel_send(1, target, EVENT_TERM, 0, 0);
        channel_wait(1, target, EVENT_DONE, 0, 0);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

