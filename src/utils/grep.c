#include <fudge.h>
#include <abi.h>

static char match[1024];
static unsigned int matchcount;

static void checkline(struct channel *channel, unsigned int source, void *buffer, unsigned int count)
{

    unsigned int i;

    if (matchcount > count)
        return;

    for (i = 0; i < count - matchcount; i++)
    {

        if (buffer_match((char *)buffer + i, match, matchcount))
        {

            channel_reply(channel, EVENT_DATA, count, buffer);

            break;

        }

    }

}

static void check(struct channel *channel, unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int start = 0;
    unsigned int i;

    matchcount = ascii_length(match);

    if (!matchcount)
        return;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n' || b[i] == '\0')
        {

            checkline(channel, source, b + start, i - start + 1);

            start = i + 1;

        }

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    check(channel, source, mdata, msize);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "match"))
        ascii_copy(match, value);

}

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_seek(FILE_L0, 0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            check(channel, source, buffer, count);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_OPTION, onoption);
    channel_setcallback(channel, EVENT_PATH, onpath);

}

