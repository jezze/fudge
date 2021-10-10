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

static unsigned int validate(void *buffer, unsigned short tag)
{

    struct p9p_header *p9p = buffer;

    if (p9p_read1(p9p, P9P_OFFSET_TYPE) == P9P_RERROR)
    {

        error(p9p_readstringdata(p9p, P9P_OFFSET_DATA), p9p_readstringlength(p9p, P9P_OFFSET_DATA));

        return 0;

    }

    if (p9p_read2(p9p, P9P_OFFSET_TAG) != tag)
    {

        char *ename = "Tags do not match";

        error(ename, cstring_length(ename));

        return 0;

    }

    return p9p_read1(p9p, P9P_OFFSET_TYPE);

}

static unsigned int version(unsigned short tag, unsigned int msize, char *name)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    file_notify(FILE_G0, EVENT_P9P, p9p_mktversion(buffer, tag, msize, name), buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (!validate(message.data.buffer, tag))
        return 0;

    switch (p9p_read1(message.data.buffer, P9P_OFFSET_TYPE))
    {

    case P9P_RVERSION:
        return 1;

    }

    return 0;

}

static unsigned int attach(unsigned short tag, unsigned int fid, unsigned int afid)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    file_notify(FILE_G0, EVENT_P9P, p9p_mktattach(buffer, tag, fid, afid, "nobody", "nobody"), buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (!validate(message.data.buffer, tag))
        return 0;

    switch (p9p_read1(message.data.buffer, P9P_OFFSET_TYPE))
    {

    case P9P_RATTACH:
        return 1;

    }

    return 0;

}

static unsigned int walk(unsigned short tag, unsigned int fid, unsigned int newfid, char *wname)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    file_notify(FILE_G0, EVENT_P9P, p9p_mktwalk(buffer, tag, fid, newfid, 1, &wname), buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (!validate(message.data.buffer, tag))
        return 0;

    switch (p9p_read1(message.data.buffer, P9P_OFFSET_TYPE))
    {

    case P9P_RWALK:
        return 1;

    }

    return 0;

}

static unsigned int read(unsigned short tag, unsigned int fid)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    file_notify(FILE_G0, EVENT_P9P, p9p_mktread(buffer, tag, fid, 0, 0, 512), buffer);
    channel_pollevent(EVENT_P9P, &message);

    if (!validate(message.data.buffer, tag))
        return 0;

    switch (p9p_read1(message.data.buffer, P9P_OFFSET_TYPE))
    {

    case P9P_RREAD:
        channel_sendbuffer(EVENT_DATA, p9p_read4(message.data.buffer, P9P_OFFSET_DATA), p9p_readbuffer(message.data.buffer, P9P_OFFSET_DATA + 4));

        return 1;

    }

    return 0;

}

static void sendrequest(void)
{

    if (!version(40, 1200, "9P2000.F"))
        return;

    if (!attach(41, 0, 0))
        return;

    if (!walk(42, 0, 1, "build/data/help.txt"))
        return;

    read(43, 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    sendrequest();
    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/fd0"))
        return;

    channel_bind(EVENT_MAIN, onmain);

}

