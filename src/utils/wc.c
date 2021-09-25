#include <fudge.h>
#include <abi.h>

static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    sum(msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    unsigned int offset = 0;

    offset = message_putvalue(&message, lines, 10, 0, offset);
    offset = message_putstring(&message, "\n", offset);
    offset = message_putvalue(&message, words, 10, 0, offset);
    offset = message_putstring(&message, "\n", offset);
    offset = message_putvalue(&message, bytes, 10, 0, offset);
    offset = message_putstring(&message, "\n", offset);

    channel_reply(EVENT_DATA, offset, message.data.buffer);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_seek(FILE_L0, 0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            sum(count, buffer);

    }

}

void init(void)
{

    channel_setcallback(EVENT_DATA, ondata);
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_PATH, onpath);

}

