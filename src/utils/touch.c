#include <fudge.h>
#include <abi.h>

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int parent = fs_walk(0, target, 0, message->data);

        if (parent)
        {

            char *name = option_getstring("name");
            unsigned int id = fs_create(0, target, parent, name, cstring_length(name));

            if (!id)
            {

                channel_send_fmt1(0, message->source, EVENT_ERROR, "File could not be created.\n", message->data);

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

    option_add("name", "");
    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

