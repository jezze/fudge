#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);
        struct record record;

        if (id && fs_stat(1, target, id, &record) && record.type != RECORD_TYPE_DIRECTORY)
        {

            channel_send_fmt(0, message->source, EVENT_ERROR, "Not a directory: %s\n", message->data);

        }

        else if (id)
        {

            char *path = message->data;
            unsigned int length = cstring_length(path);
            char *slash = (length && path[length - 1] != '/' && path[length - 1] != ':') ? "/" : "";

            channel_send_fmt(0, message->source, EVENT_OPTION, "pwd=%s%s\n", path, slash);

        }

        else
        {

            channel_send_fmt(0, message->source, EVENT_ERROR, "Directory not found: %s\n", message->data);

        }

    }

    else
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Service not found: %s\n", message->data);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

