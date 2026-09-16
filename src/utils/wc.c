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

static void ondata(struct message *message)
{

    sum(message->length, message->data);

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, message->data);

        if (id)
        {

            char buffer[4096];
            unsigned int count;
            unsigned int offset;

            for (offset = 0; (count = fs_read(1, target, id, buffer, 4096, offset)); offset += count)
                sum(count, buffer);

        }

        else
        {

            channel_send_fmt1(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        }

    }

}

static void onterm(struct message *message)
{

    channel_send_fmt3(0, message->source, EVENT_DATA, "%u\n%u\n%u\n", &lines, &words, &bytes);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

    while (channel_process(0));

}

