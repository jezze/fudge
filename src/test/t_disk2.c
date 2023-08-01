#include <fudge.h>
#include <abi.h>

static void error(void *data, unsigned int count)
{

    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_ERROR, "Error occured:\n%w\n", data, &count);

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
    char data[MESSAGE_SIZE];

    call_notify(FILE_G0, EVENT_P9P, p9p_mktversion(buffer, tag, msize, name), buffer);
    channel_poll_any(EVENT_P9P, &message, data);

    if (!validate(data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
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
    char data[MESSAGE_SIZE];

    call_notify(FILE_G0, EVENT_P9P, p9p_mktattach(buffer, tag, fid, afid, "nobody", "nobody"), buffer);
    channel_poll_any(EVENT_P9P, &message, data);

    if (!validate(data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
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
    char data[MESSAGE_SIZE];

    call_notify(FILE_G0, EVENT_P9P, p9p_mktwalk(buffer, tag, fid, newfid, 1, &wname), buffer);
    channel_poll_any(EVENT_P9P, &message, data);

    if (!validate(data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
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
    char data[MESSAGE_SIZE];

    call_notify(FILE_G0, EVENT_P9P, p9p_mktread(buffer, tag, fid, 0, 0, 512), buffer);
    channel_poll_any(EVENT_P9P, &message, data);

    if (!validate(data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
    {

    case P9P_RREAD:
        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, p9p_read4(data, P9P_OFFSET_DATA), p9p_readbuffer(data, P9P_OFFSET_DATA + 4));

        return 1;

    }

    return 0;

}

static void sendrequest(void)
{

    if (!version(40, 1200, "9P2000.F"))
        channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Unrcognized version\n");

    if (!attach(41, 0, 0))
        channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Attach failed\n");

    if (!walk(42, 0, 1, "build/data/help.txt"))
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "File not found: %s\n", "build/data/help.txt");

    read(43, 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    sendrequest();

}

void init(void)
{

    call_walk_absolute(FILE_G0, "system:service/fd0");
    channel_bind(EVENT_MAIN, onmain);

}

