#include <fudge.h>
#include <abi.h>

static unsigned int page;

static void print(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        char data[120];
        unsigned int offset = 0;
        unsigned int j;

        offset += cstring_write_fmt(data, 120, offset, "%H8u  ", &page);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
                offset += cstring_write_fmt(data, 120, offset, "%H2c ", &b[j]);
            else
                offset += cstring_write_fmt(data, 120, offset, "   ");

        }

        offset += cstring_write_fmt(data, 120, offset, " |");

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

                offset += cstring_write_fmt(data, 120, offset, " ");

            }

        }

        offset += cstring_write_fmt(data, 120, offset, "|\n");
        page += 16;

        channel_send(0, source, EVENT_DATA, offset, data);

    }

}

static void ondata(struct message *message)
{

    print(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    page = 0;

    fs_read_each(1, message->source, message->data, print);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

