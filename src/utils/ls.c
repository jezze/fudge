#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void list(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    unsigned char data[MESSAGE_SIZE];
    unsigned int count;
    unsigned int offset = 0;

    if (!id)
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Path not found: %s\n", path);

        return;

    }

    while ((count = fs_read(1, target, id, data, MESSAGE_SIZE, offset)))
    {

        unsigned int i;

        for (i = 0; i < count; i += sizeof (struct record))
        {

            struct record *record = (struct record *)(data + i);

            if (record->type == RECORD_TYPE_DIRECTORY)
                channel_send_fmt(0, source, MESSAGE_TYPE_DATA, "%w/\n", record->name, &record->length);
            else
                channel_send_fmt(0, source, MESSAGE_TYPE_DATA, "%w\n", record->name, &record->length);

            offset = record->offset;

        }

    }

}

static void onpath(struct message *message)
{

    paths++;

    list(message->source, message->data);

}

static void onterm(struct message *message)
{

    if (!paths)
        list(message->source, option_getstring("pwd"));

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_PATH, onpath);
    channel_bind(MESSAGE_TYPE_TERM, onterm);

}

