#include <fudge.h>
#include <abi.h>

static unsigned int paths;

static void create(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int parent = fs_walk(0, target, 0, path);

        if (parent)
        {

            char *name = option_getstring("name");
            unsigned int id = fs_create(0, target, parent, name, cstring_length(name));

            if (!id)
            {

                channel_send_fmt0(0, source, EVENT_ERROR, "File could not be created.\n");

            }

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        }

    }

}

static void onpath(struct message *message)
{

    paths++;
    create(message->source, message->data);

}

static void onterm(struct message *message)
{

    if (!paths)
        create(message->source, option_getstring("pwd"));

}

void init(void)
{

    option_add("name", "");
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

