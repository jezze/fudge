#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fs_auth(mdata);

    if (service)
    {

        unsigned int id = fs_walk(0, service, 0, mdata);

        if (id)
            call_unload(fs_map(0, service, id));
        else
            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

