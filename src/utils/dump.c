#include <fudge.h>
#include <abi.h>

static unsigned int page;

static void print(unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        struct message message;
        unsigned int j;

        message_init(&message, EVENT_DATA);
        message_putvalue(&message, page, 16, 8);
        message_putstring(&message, "  ");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                message_putvalue(&message, b[j], 16, 2);
                message_putstring(&message, " ");

            }

            else
            {

                message_putstring(&message, "   ");

            }

        }

        message_putstring(&message, " |");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = b[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                message_putbuffer(&message, 1, &c);

            }

            else
            {

                message_putstring(&message, " ");

            }

        }

        message_putstring(&message, "|\n");
        channel_sendmessage(&message);

        page += 16;

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    print(source, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    page = 0;

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_reset(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            print(source, count, buffer);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

