#include <fudge.h>
#include <abi.h>

static void error(struct event_p9p *p9p, void *data)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Error occured:\n");
    message_putbuffer(&message, p9p_read4(p9p->size) - sizeof (struct event_p9p), data);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static unsigned int sendandpoll(struct message *request, struct message *response)
{

    struct event_p9p *p9p = (struct event_p9p *)response->data.buffer;

    file_notify(FILE_G0, EVENT_P9P, message_datasize(&request->header), request->data.buffer);
    channel_pollevent(EVENT_P9P, response);

    if (p9p_read1(p9p->type) == P9P_RERROR)
    {

        error(p9p, p9p + 1);

        return 0;

    }

    return p9p_read1(p9p->type);

}

static unsigned int version(unsigned int msize, char *name)
{

    struct message request;
    struct message response;

    p9p_mktversion(&request, msize, name);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RVERSION:
        return 1;

    }

    return 0;

}

static unsigned int walk(unsigned int fid, unsigned int newfid, char *wname)
{

    struct message request;
    struct message response;

    p9p_mktwalk(&request, fid, newfid, wname);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RWALK:
        return 1;

    }

    return 0;

}

static unsigned int read(void)
{

    struct message request;
    struct message response;

    p9p_mktread(&request, 22445566, 0, 0, 512);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RREAD:
        channel_sendbuffer(EVENT_DATA, message_datasize(&response.header) - sizeof (struct event_p9p) - sizeof (struct p9p_rread), response.data.buffer + sizeof (struct event_p9p) + sizeof (struct p9p_rread));

        return 1;

    }

    return 0;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (version(1200, "9P2000"))
    {

        if (walk(22445566, 0, "build/data/help.txt"))
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

