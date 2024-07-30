#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fsp_auth(mdata);

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, mdata);

        if (id)
            channel_send_fmt1(source, EVENT_OPTION, "pwd\\0%s\\0", mdata);
        else
            channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

