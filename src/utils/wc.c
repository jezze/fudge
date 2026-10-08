#include <fudge.h>
#include <abi.h>

static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(unsigned int source, void *buffer, unsigned int count)
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

static void ondata(struct message *message)
{

    sum(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    fs_read_each(1, message->source, message->data, sum);

}

static void onterm(struct message *message)
{

    channel_send_fmt(0, message->source, EVENT_DATA, "%u\n%u\n%u\n", &lines, &words, &bytes);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

