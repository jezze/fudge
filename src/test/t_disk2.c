#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    p9p_walk(&message, "build/data/help.txt");
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_p9p *p9p = mdata;
    struct message message;

    switch (p9p_read1(p9p->type))
    {

    case P9P_RWALK:
        p9p_read(&message);
        file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);

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

