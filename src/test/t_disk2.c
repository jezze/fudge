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

    struct p9p_event *p9prequest = (struct p9p_event *)request->data.buffer;
    struct p9p_event *p9presponse = (struct p9p_event *)response->data.buffer;

    file_notify(FILE_G0, EVENT_P9P, message_datasize(&request->header), request->data.buffer);
    channel_pollevent(EVENT_P9P, response);

    if (p9p_read1(p9presponse, P9P_OFFSET_TYPE) == P9P_RERROR)
    {

        error(response->data.buffer + P9P_OFFSET_DATA + 2, p9p_read2(p9presponse, P9P_OFFSET_DATA));

        return 0;

    }

    if (p9p_read2(p9prequest, P9P_OFFSET_TAG) != p9p_read2(p9presponse, P9P_OFFSET_TAG))
    {

        char *errmsg = "Tags do not match";

        error(errmsg, ascii_length(errmsg));

        return 0;

    }

    return p9p_read1(p9presponse, P9P_OFFSET_TYPE);

}

static unsigned int version(unsigned int msize, char *name)
{

    char buffer[MESSAGE_SIZE];
    struct message request;
    struct message response;

    message_init(&request, EVENT_P9P);
    message_putbuffer(&request, p9p_mktversion(buffer, 41, msize, name), buffer);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RVERSION:
        return 1;

    }

    return 0;

}

static unsigned int attach(unsigned int fid, unsigned int afid)
{

    char buffer[MESSAGE_SIZE];
    struct message request;
    struct message response;

    message_init(&request, EVENT_P9P);
    message_putbuffer(&request, p9p_mktattach(buffer, 45, fid, afid, "nobody", "nobody"), buffer);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RATTACH:
        return 1;

    }

    return 0;

}

static unsigned int walk(unsigned int fid, unsigned int newfid, char *wname)
{

    char buffer[MESSAGE_SIZE];
    struct message request;
    struct message response;

    message_init(&request, EVENT_P9P);
    message_putbuffer(&request, p9p_mktwalk(buffer, 42, fid, newfid, 1, &wname), buffer);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RWALK:
        return 1;

    }

    return 0;

}

static unsigned int read(unsigned int fid)
{

    char buffer[MESSAGE_SIZE];
    struct message request;
    struct message response;

    message_init(&request, EVENT_P9P);
    message_putbuffer(&request, p9p_mktread(buffer, 43, fid, 0, 0, 512), buffer);

    switch (sendandpoll(&request, &response))
    {

    case P9P_RREAD:
        channel_sendbuffer(EVENT_DATA, p9p_read4(response.data.buffer, P9P_OFFSET_DATA), response.data.buffer + P9P_OFFSET_DATA + 4);

        return 1;

    }

    return 0;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (version(1200, "9P2000"))
    {

        if (attach(1, 1))
        {

            if (walk(1, 1, "build/data/help.txt"))
            {

                if (read(1))
                {

                }

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

