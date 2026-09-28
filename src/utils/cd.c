#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
            channel_send_fmt1(0, message->source, EVENT_OPTION, "pwd=%s\n", message->data);
        else
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Directory not found: %s\n", message->data);

    }

    else
    {

        channel_send_fmt1(0, message->source, EVENT_ERROR, "Service not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

