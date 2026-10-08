#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int event = option_getdecimal("event");
    unsigned int target = fs_auth(message->data);
    unsigned int id = (target) ? fs_walk(1, target, 0, message->data) : 0;
    char data[MESSAGE_SIZE];
    unsigned int offset = 0;

    if (!id)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        return;

    }

    if (!event)
        event = EVENT_DATA;

    if (event == EVENT_DATA || event == EVENT_ERROR)
    {

        unsigned int count;

        while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
        {

            channel_send(0, message->source, event, count, data);

            offset += count;

        }

    }

    else
    {

        unsigned int header[2];

        while (fs_read_full(1, target, id, header, 8, offset) == 8 && header[1] <= MESSAGE_SIZE)
        {

            if (fs_read_full(1, target, id, data, header[1], offset + 8) != header[1])
                break;

            channel_send(0, message->source, event, header[1], data);

            offset += 8 + header[1];

        }

    }

}

void init(void)
{

    option_add("event", "0");
    channel_bind(EVENT_PATH, onpath);

}

