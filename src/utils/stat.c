#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            struct record record;

            if (fs_stat(1, target, id, &record))
            {

                channel_send_fmt1(0, message->source, EVENT_DATA, "Id: %u\n", &record.id);
                channel_send_fmt2(0, message->source, EVENT_DATA, "Name: %w\n", record.name, &record.length);
                channel_send_fmt1(0, message->source, EVENT_DATA, "Type: %u\n", &record.type);
                channel_send_fmt1(0, message->source, EVENT_DATA, "Size: %u\n", &record.size);

            }

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

