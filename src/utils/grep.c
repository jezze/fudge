#include <fudge.h>
#include <abi.h>

static void checkprefix(unsigned int source, void *buffer, unsigned int count)
{

    char *prefix = option_getstring("prefix");
    unsigned int prefixcount = cstring_length(prefix);

    if (prefixcount && prefixcount <= count)
    {

        if (buffer_match(buffer, prefix, prefixcount))
            channel_send_buffer(0, source, EVENT_DATA, count, buffer);

    }

}

static void checksubstr(unsigned int source, void *buffer, unsigned int count)
{

    char *substr = option_getstring("substr");
    unsigned int substrcount = cstring_length(substr);

    if (substrcount && substrcount <= count)
    {

        unsigned int i;

        for (i = 0; i < count - substrcount; i++)
        {

            if (buffer_match((char *)buffer + i, substr, substrcount))
            {

                channel_send_buffer(0, source, EVENT_DATA, count, buffer);

                break;

            }

        }

    }

}

static void check(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n' || b[i] == '\0')
        {

            checkprefix(source, b + start, i - start + 1);
            checksubstr(source, b + start, i - start + 1);

            start = i + 1;

        }

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    check(source, mdata, msize);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, mdata);

        if (id)
        {

            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
                check(source, buffer, count);

        }

    }

}

void init(void)
{

    option_add("prefix", "");
    option_add("substr", "");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

