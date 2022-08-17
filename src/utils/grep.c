#include <fudge.h>
#include <abi.h>

static char match[1024];
static unsigned int matchcount;

static void checkline(unsigned int source, void *buffer, unsigned int count)
{

    unsigned int i;

    if (matchcount > count)
        return;

    for (i = 0; i < count - matchcount; i++)
    {

        if (buffer_match((char *)buffer + i, match, matchcount))
        {

            channel_sendbuffer(EVENT_DATA, count, buffer);

            break;

        }

    }

}

static void check(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int start = 0;
    unsigned int i;

    matchcount = cstring_length(match);

    if (!matchcount)
        return;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n' || b[i] == '\0')
        {

            checkline(source, b + start, i - start + 1);

            start = i + 1;

        }

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    check(source, mdata, msize);

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "match"))
        cstring_copy(match, value);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            check(source, buffer, count);

    }

    else
    {

        channel_error("File not found");

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_PATH, onpath);

}

