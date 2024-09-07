#include <fudge.h>
#include <abi.h>

static unsigned int eachchar(char *data, unsigned int length, unsigned int offset, char c)
{

    unsigned int i;

    for (i = offset; i < length; i++)
    {

        if (data[i] == c || data[i] == '\0')
            return i + 1 - offset;

    }

    return 0;

}

static void cleanpath(unsigned int source, char *data, unsigned int count, unsigned int start)
{

    unsigned int offset;
    unsigned int length;

    for (offset = start; (length = eachchar(data, count, offset, '/')); offset += length)
    {

        unsigned int nextoffset = offset + length;

        if (length == 1 && buffer_match(data + offset, "/", 1))
        {

            buffer_copy(data + offset, data + nextoffset, count - nextoffset);

            length = 0;

        }

        else if (length == 2 && buffer_match(data + offset, ".", 1))
        {

            buffer_copy(data + offset, data + nextoffset, count - nextoffset);

            length = 0;

        }

        else if (length == 3 && buffer_match(data + offset, "..", 2))
        {

            buffer_copy(data + offset, data + nextoffset - 1, count - nextoffset + 1);

            length = 0;

        }

        else
        {

            unsigned int nextlength = eachchar(data, count, nextoffset, '/');

            if (nextlength == 3 && buffer_match(data + nextoffset, "..", 2))
            {

                nextoffset += nextlength;

                buffer_copy(data + offset, data + nextoffset - 1, count - nextoffset + 1);

                length = 0;
                offset = start;

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int start = eachchar(mdata, msize, 0, ':');
    unsigned int service;

    cleanpath(source, mdata, msize, start);

    service = fs_auth(mdata);

    if (service)
    {

        unsigned int id = fs_walk(service, 0, mdata);

        if (id)
            channel_send_fmt1(source, EVENT_OPTION, "pwd\\0%s\\0", mdata);
        else
            channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

