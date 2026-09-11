#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);

    if (target)
    {

        unsigned int parent = fs_walk(0, target, 0, mdata);

        if (parent)
        {

            char *name = option_getstring("name");
            unsigned int id = fs_create(0, target, parent, name, cstring_length(name));

            if (!id)
            {

                channel_send_fmt1(0, source, EVENT_ERROR, "File could not be created.\n", mdata);

            }

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    option_add("name", "");
    channel_bind(EVENT_PATH, onpath);

    while (channel_process(0));

}

