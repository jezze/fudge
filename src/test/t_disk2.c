#include <fudge.h>
#include <abi.h>

static void error(unsigned int source, void *data, unsigned int count)
{

    channel_send_fmt2(0, source, EVENT_ERROR, "Error occured:\n%w\n", data, &count);

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

static unsigned int version(unsigned int target, unsigned int source, unsigned short tag, unsigned int msize, char *name)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    channel_send(0, target, EVENT_P9P, p9p_mktversion(buffer, tag, msize, name), buffer);
    channel_poll(0, target, EVENT_P9P, &message);

    if (!validate(source, message_data(&message, 0), tag))
        return 0;

    switch (p9p_read1(message_data(&message, 0), P9P_OFFSET_TYPE))
    {

    case P9P_RVERSION:
        return 1;

    }

    return 0;

}

static unsigned int attach(unsigned int target, unsigned int source, unsigned short tag, unsigned int fid, unsigned int afid)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    channel_send(0, target, EVENT_P9P, p9p_mktattach(buffer, tag, fid, afid, "nobody", "nobody"), buffer);
    channel_poll(0, target, EVENT_P9P, &message);

    if (!validate(source, message_data(&message, 0), tag))
        return 0;

    switch (p9p_read1(message_data(&message, 0), P9P_OFFSET_TYPE))
    {

    case P9P_RATTACH:
        return 1;

    }

    return 0;

}

static unsigned int walk(unsigned int target, unsigned int source, unsigned short tag, unsigned int fid, unsigned int newfid, char *wname)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    channel_send(0, target, EVENT_P9P, p9p_mktwalk(buffer, tag, fid, newfid, 1, &wname), buffer);
    channel_poll(0, target, EVENT_P9P, &message);

    if (!validate(source, message_data(&message, 0), tag))
        return 0;

    switch (p9p_read1(message_data(&message, 0), P9P_OFFSET_TYPE))
    {

    case P9P_RWALK:
        return 1;

    }

    return 0;

}

static unsigned int read(unsigned int target, unsigned int source, unsigned short tag, unsigned int fid)
{

    char buffer[MESSAGE_SIZE];
    struct message message;

    channel_send(0, target, EVENT_P9P, p9p_mktread(buffer, tag, fid, 0, 0, 512), buffer);
    channel_poll(0, target, EVENT_P9P, &message);

    if (!validate(source, message_data(&message, 0), tag))
        return 0;

    switch (p9p_read1(message_data(&message, 0), P9P_OFFSET_TYPE))
    {

    case P9P_RREAD:
        channel_send(0, source, EVENT_DATA, p9p_read4(message_data(&message, 0), P9P_OFFSET_DATA), p9p_readbuffer(message_data(&message, 0), P9P_OFFSET_DATA + 4));

        return 1;

    }

    return 0;

}

static void sendrequest(unsigned int target, unsigned int source)
{

    if (!version(target, source, 40, 1200, "9P2000.F"))
        channel_send_fmt0(0, source, EVENT_ERROR, "Unrcognized version\n");

    if (!attach(target, source, 41, 0, 0))
        channel_send_fmt0(0, source, EVENT_ERROR, "Attach failed\n");

    if (!walk(target, source, 42, 0, 1, option_getstring("path")))
        channel_send_fmt1(0, source, EVENT_ERROR, "File not found: %s\n", option_getstring("path"));

    read(target, source, 43, 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = channel_lookup(option_getstring("9p-service"));

    sendrequest(target, source);

}

void init(void)
{

    option_add("9p-service", "9p");
    option_add("path", "build/data/help.txt");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

