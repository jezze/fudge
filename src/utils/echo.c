#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(source, EVENT_DATA, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fs_auth(mdata);

    if (service)
    {

        unsigned int id = fs_walk(service, 0, mdata);

        if (id)
        {

            char buffer[MESSAGE_SIZE];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(service, id, buffer, MESSAGE_SIZE, offset)); offset += count)
                channel_send_buffer(source, EVENT_DATA, count, buffer);

        }

        else
        {

            channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

