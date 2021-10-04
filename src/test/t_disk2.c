#include <fudge.h>
#include <abi.h>

static unsigned int version(void)
{

    struct message message;
    struct event_p9p *p9p = (struct event_p9p *)message.data.buffer;

    p9p_mktversion(&message, 1200, "9P2000");
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (p9p_read1(p9p->type) == P9P_RERROR)
        return 0;

    if (p9p_read1(p9p->type) == P9P_RVERSION)
        return 1;

    return 0;

}

static unsigned int walk(void)
{

    struct message message;
    struct event_p9p *p9p = (struct event_p9p *)message.data.buffer;

    p9p_mktwalk(&message, 22445566, 0, "build/data/help.txt");
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (p9p_read1(p9p->type) == P9P_RERROR)
        return 0;

    if (p9p_read1(p9p->type) == P9P_RWALK)
        return 1;

    return 0;

}

static unsigned int read(void)
{

    struct message message;
    struct event_p9p *p9p = (struct event_p9p *)message.data.buffer;

    p9p_mktread(&message, 22445566, 0, 0, 512);
    file_notify(FILE_G0, EVENT_P9P, message_datasize(&message.header), message.data.buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (p9p_read1(p9p->type) == P9P_RERROR)
        return 0;

    /*
    if (p9p_read1(p9p->type) == P9P_RREAD)
        return 0;
    */

    channel_sendbuffer(EVENT_DATA, message_datasize(&message.header) - sizeof (struct event_p9p) - sizeof (struct p9p_rread), message.data.buffer + sizeof (struct event_p9p) + sizeof (struct p9p_rread));

    return 0;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (version())
    {

        if (walk())
        {

            if (read())
            {

            }

        }

    }

    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/fd0"))
        return;

    channel_bind(EVENT_MAIN, onmain);

}

