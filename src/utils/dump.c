#include <fudge.h>
#include <abi.h>

static unsigned int page;

static void print(unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        char data[120];
        unsigned int offset = 0;
        unsigned int j;

        offset += cstring_write_fmt1(data, 120, offset, "%H8u  ", &page);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
                offset += cstring_write_fmt1(data, 120, offset, "%H2c ", &b[j]);
            else
                offset += cstring_write_fmt0(data, 120, offset, "   ");

        }

        offset += cstring_write_fmt0(data, 120, offset, " |");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = b[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                offset += buffer_write(data, 120, &c, 1, offset);

            }

            else
            {

                offset += cstring_write_fmt0(data, 120, offset, " ");

            }

        }

        offset += cstring_write_fmt0(data, 120, offset, "|\n");
        page += 16;

        channel_send_buffer(0, source, EVENT_DATA, offset, data);

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(source, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, mdata);

        page = 0;

        if (id)
        {

            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
                print(source, count, buffer);

        }

        else
        {

            channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

