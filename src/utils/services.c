#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int target = fs_auth("namesrv:");
    unsigned int id = (target) ? fs_walk(1, target, 0, "namesrv:") : 0;
    unsigned char data[MESSAGE_SIZE];
    unsigned int count;
    unsigned int offset = 0;

    if (!id)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Service not found: namesrv\n");

        return;

    }

    channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "{\n");
    channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "  services: [\n");

    while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
    {

        unsigned int i;

        for (i = 0; i < count; i += sizeof (struct record))
        {

            struct record *record = (struct record *)(data + i);

            offset = record->offset;

            if (record->name[0] == '.')
                continue;

            channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "    {\n");
            channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "      name: \"%w\"\n", record->name, &record->length);
            channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "    }\n");

        }

    }

    channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "  ]\n");
    channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "}\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

