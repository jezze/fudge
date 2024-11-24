#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(0, source, EVENT_DATA, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, mdata);

        if (id)
        {

            char buffer[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, MESSAGE_SIZE, offset)); offset += count)
                channel_send_buffer(0, source, EVENT_DATA, count, buffer);

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

