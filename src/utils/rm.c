#include <fudge.h>
#include <abi.h>

static unsigned int walkparent(unsigned int ichannel, unsigned int target, char *path)
{

    char parent[1024];
    unsigned int length = 0;
    unsigned int i;

    for (i = 0; path[i] && i < 1023; i++)
    {

        if (path[i] == '/' || path[i] == ':')
            length = i + 1;

    }

    buffer_write(parent, 1024, path, length, 0);

    parent[length] = '\0';

    return fs_walk(ichannel, target, 0, parent);

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(0, target, 0, message->data);

        if (id)
        {

            if (!fs_remove(0, target, walkparent(0, target, message->data), id))
                channel_send_fmt(0, message->source, EVENT_ERROR, "File could not be removed: %s\n", message->data);

        }

        else
        {

            channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

