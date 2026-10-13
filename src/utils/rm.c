#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    char *path = message->data;
    unsigned int length = cstring_length(path);
    unsigned int target;

    if (length && path[length - 1] == '/')
        path[length - 1] = '\0';

    target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            if (!fs_remove(1, target, fs_walkparent(1, target, path), id))
                channel_send_fmt(0, message->source, EVENT_ERROR, "File could not be removed: %s\n", path);

        }

        else
        {

            channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", path);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

