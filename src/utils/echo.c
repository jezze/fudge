#include <fudge.h>
#include <abi.h>

static void ondata(struct message *message)
{

    channel_send(0, message->source, EVENT_DATA, message->length, message->data);

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char buffer[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, MESSAGE_SIZE, offset)); offset += count)
                channel_send(0, message->source, EVENT_DATA, count, buffer);

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

