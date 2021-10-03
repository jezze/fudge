#include <fudge.h>
#include <abi.h>

static void p9p_walk(unsigned int descriptor, unsigned int source)
{

    struct event_p9p header;

    header.type = P9P_TWALK;

    file_notify(descriptor, EVENT_P9P, sizeof (struct event_p9p), &header);

}

static void p9p_read(unsigned int descriptor, unsigned int source)
{

    struct event_p9p header;

    header.type = P9P_TREAD;

    file_notify(descriptor, EVENT_P9P, sizeof (struct event_p9p), &header);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    p9p_walk(FILE_G0, source);

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_p9p *p9p = mdata;

    switch (p9p->type)
    {

    case P9P_RWALK:
        p9p_read(FILE_G0, source);

        break;

    case P9P_RREAD:
        channel_sendbuffer(EVENT_DATA, msize - sizeof (struct event_p9p), p9p + 1);
        channel_close();

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

