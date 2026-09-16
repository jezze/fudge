#include <fudge.h>
#include <abi.h>
#include <binary.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
            call_unload(fs_map(1, target, id));
        else
            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

