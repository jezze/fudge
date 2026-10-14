#include <fudge.h>
#include <abi.h>

static unsigned int lines;

static void output(unsigned int source, void *buffer, unsigned int count)
{

    char *data = buffer;
    unsigned int max = option_getdecimal("n");
    unsigned int i;

    for (i = 0; i < count && lines < max; i++)
    {

        if (data[i] == '\n')
            lines++;

    }

    if (i)
        channel_send(0, source, EVENT_DATA, i, data);

}

static void ondata(struct message *message)
{

    output(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    lines = 0;

    fs_read_each(1, message->source, message->data, output);

}

void init(void)
{

    option_add("n", "10");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

