#include <fudge.h>
#include <abi.h>

static void error(void *data, unsigned int count)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Error occured:\n");
    message_putbuffer(&message, count, data);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static unsigned int sendandpoll(struct message *request, struct message *response)
{

    struct event_p9p *p9prequest = (struct event_p9p *)request->data.buffer;
    struct event_p9p *p9presponse = (struct event_p9p *)response->data.buffer;

    file_notify(FILE_G0, EVENT_P9P, message_datasize(&request->header), request->data.buffer);
    channel_pollevent(EVENT_P9P, response);

    if (p9p_read1(p9presponse->type) == P9P_RERROR)
    {

        error(p9presponse + 1, p9p_read4(p9presponse->size) - sizeof (struct event_p9p));

        return 0;

    }

    if (p9p_read2(p9prequest->tag) != p9p_read2(p9presponse->tag))
    {

        char *errmsg = "Tags do not match";

        error(errmsg, ascii_length(errmsg));

        return 0;

    }

    return p9p_read1(p9presponse->type);

}

static unsigned int version(unsigned int msize, char *name)
{

    struct message request;
    struct message response;

    p9p_mktversion(&request, 41, msize, name);

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

    p9p_mktwalk(&request, 42, fid, newfid, wname);

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

    p9p_mktread(&request, 43, 22445566, 0, 0, 512);

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

