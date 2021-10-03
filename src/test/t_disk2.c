#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_p9p auth;

    auth.type = P9P_TAUTH;

    file_notify(FILE_G0, EVENT_P9P, sizeof (struct event_p9p), &auth);

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_p9p *p9p = mdata;

    switch (p9p->type)
    {

    case P9P_RAUTH:
        channel_sendstring(EVENT_DATA, "Auth message received!\n");

        break;

    default:
        channel_sendstring(EVENT_DATA, "Unknown message received!\n");

        break;

    }

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/fd0"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_P9P, onp9p);

}

