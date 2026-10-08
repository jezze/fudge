#include <fudge.h>
#include <abi.h>

static unsigned int event;

static void senddata(unsigned int source, void *buffer, unsigned int count)
{

    channel_send(0, source, event, count, buffer);

}

static void onpath(struct message *message)
{

    unsigned int target;
    unsigned int id;
    char data[MESSAGE_SIZE];
    unsigned int header[2];
    unsigned int offset = 0;

    event = option_getdecimal("event");

    if (!event || event == EVENT_DATA || event == EVENT_ERROR)
    {

        event = (event) ? event : EVENT_DATA;

        fs_read_each(1, message->source, message->data, senddata);

        return;

    }

    target = fs_auth(message->data);
    id = (target) ? fs_walk(1, target, 0, message->data) : 0;

    if (!id)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        return;

    }

    while (fs_read_full(1, target, id, header, 8, offset) == 8 && header[1] <= MESSAGE_SIZE)
    {

        if (fs_read_full(1, target, id, data, header[1], offset + 8) != header[1])
            break;

        channel_send(0, message->source, event, header[1], data);

        offset += 8 + header[1];

    }

}

void init(void)
{

    option_add("event", "0");
    channel_bind(EVENT_PATH, onpath);

}

