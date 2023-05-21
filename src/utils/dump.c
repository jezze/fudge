#include <fudge.h>
#include <abi.h>

static unsigned int page;

static void print(unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        char buffer[120];
        unsigned int offset = 0;
        unsigned int j;

        offset += cstring_write_fmt1(buffer, 120, "%H8u  ", offset, &page);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
                offset += cstring_write_fmt1(buffer, 120, "%H2c ", offset, &b[j]);
            else
                offset += cstring_write_fmt0(buffer, 120, "   ", offset);

        }

        offset += cstring_write_fmt0(buffer, 120, " |", offset);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = b[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                offset += buffer_write(buffer, 120, &c, 1, offset);

            }

            else
            {

                offset += cstring_write_fmt0(buffer, 120, " ", offset);

            }

        }

        offset += cstring_write_fmt0(buffer, 120, "|\n", offset);
        page += 16;

        channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, offset, buffer);

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(source, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    page = 0;

    if (call_walk_absolute(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;
        unsigned int offset;

        for (offset = 0; (count = call_read(FILE_L0, buffer, BUFFER_SIZE, offset)); offset += count)
            print(source, count, buffer);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

