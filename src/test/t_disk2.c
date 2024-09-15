#include <fudge.h>
#include <abi.h>

static void error(unsigned int source, void *data, unsigned int count)
{

    channel_send_fmt2(source, EVENT_ERROR, "Error occured:\n%w\n", data, &count);

}

static unsigned int validate(unsigned int source, void *buffer, unsigned short tag)
{

    struct p9p_header *p9p = buffer;

    if (p9p_read1(p9p, P9P_OFFSET_TYPE) == P9P_RERROR)
    {

        error(source, p9p_readstringdata(p9p, P9P_OFFSET_DATA), p9p_readstringlength(p9p, P9P_OFFSET_DATA));

        return 0;

    }

    if (p9p_read2(p9p, P9P_OFFSET_TAG) != tag)
    {

        char *ename = "Tags do not match";

        error(source, ename, cstring_length(ename));

        return 0;

    }

    return p9p_read1(p9p, P9P_OFFSET_TYPE);

}

static unsigned int version(unsigned int source, unsigned short tag, unsigned int msize, char *name)
{

    char data[MESSAGE_SIZE];

    channel_send_buffer(option_getdecimal("9p-service"), EVENT_P9P, p9p_mktversion(data, tag, msize, name), data);
    channel_wait_buffer(option_getdecimal("9p-service"), EVENT_P9P, MESSAGE_SIZE, data);

    if (!validate(source, data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
    {

    case P9P_RVERSION:
        return 1;

    }

    return 0;

}

static unsigned int attach(unsigned int source, unsigned short tag, unsigned int fid, unsigned int afid)
{

    char buffer[MESSAGE_SIZE];
    char data[MESSAGE_SIZE];

    channel_send_buffer(option_getdecimal("9p-service"), EVENT_P9P, p9p_mktattach(buffer, tag, fid, afid, "nobody", "nobody"), buffer);
    channel_wait_buffer(option_getdecimal("9p-service"), EVENT_P9P, MESSAGE_SIZE, data);

    if (!validate(source, data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
    {

    case P9P_RATTACH:
        return 1;

    }

    return 0;

}

static unsigned int walk(unsigned int source, unsigned short tag, unsigned int fid, unsigned int newfid, char *wname)
{

    char data[MESSAGE_SIZE];

    channel_send_buffer(option_getdecimal("9p-service"), EVENT_P9P, p9p_mktwalk(data, tag, fid, newfid, 1, &wname), data);
    channel_wait_buffer(option_getdecimal("9p-service"), EVENT_P9P, MESSAGE_SIZE, data);

    if (!validate(source, data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
    {

    case P9P_RWALK:
        return 1;

    }

    return 0;

}

static unsigned int read(unsigned int source, unsigned short tag, unsigned int fid)
{

    char data[MESSAGE_SIZE];

    channel_send_buffer(option_getdecimal("9p-service"), EVENT_P9P, p9p_mktread(data, tag, fid, 0, 0, 512), data);
    channel_wait_buffer(option_getdecimal("9p-service"), EVENT_P9P, MESSAGE_SIZE, data);

    if (!validate(source, data, tag))
        return 0;

    switch (p9p_read1(data, P9P_OFFSET_TYPE))
    {

    case P9P_RREAD:
        channel_send_buffer(source, EVENT_DATA, p9p_read4(data, P9P_OFFSET_DATA), p9p_readbuffer(data, P9P_OFFSET_DATA + 4));

        return 1;

    }

    return 0;

}

static void sendrequest(unsigned int source)
{

    if (!version(source, 40, 1200, "9P2000.F"))
        channel_send_fmt0(source, EVENT_ERROR, "Unrcognized version\n");

    if (!attach(source, 41, 0, 0))
        channel_send_fmt0(source, EVENT_ERROR, "Attach failed\n");

    if (!walk(source, 42, 0, 1, option_getstring("path")))
        channel_send_fmt1(source, EVENT_ERROR, "File not found: %s\n", option_getstring("path"));

    read(source, 43, 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    sendrequest(source);

}

void init(void)
{

    option_add("9p-service", "5588");
    option_add("path", "build/data/help.txt");
    channel_bind(EVENT_MAIN, onmain);

}

