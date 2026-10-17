#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);
    unsigned int id = (target) ? fs_walk(1, target, 0, message->data) : 0;

    if (id)
    {

        struct record record;

        if (fs_stat(1, target, id, &record))
        {

            channel_send_fmt(0, message->source, EVENT_DATA, "{\n");
            channel_send_fmt(0, message->source, EVENT_DATA, "  id: %u\n", &record.id);
            channel_send_fmt(0, message->source, EVENT_DATA, "  name: \"%w\"\n", record.name, &record.length);
            channel_send_fmt(0, message->source, EVENT_DATA, "  type: %s\n", (record.type == RECORD_TYPE_DIRECTORY) ? "directory" : "file");
            channel_send_fmt(0, message->source, EVENT_DATA, "  size: %u\n", &record.size);
            channel_send_fmt(0, message->source, EVENT_DATA, "}\n");

        }

    }

    else
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

