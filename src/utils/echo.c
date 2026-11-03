#include <fudge.h>
#include <abi.h>

static void output(unsigned int source, void *buffer, unsigned int count)
{

    channel_send(0, source, MESSAGE_DATA, count, buffer);

}

static void ondata(struct message *message)
{

    output(message->source, message->data, message->length);

}

static void onpath(struct message *message)
{

    fs_read_each(1, message->source, message->data, output);

}

void init(void)
{

    channel_bind(MESSAGE_DATA, ondata);
    channel_bind(MESSAGE_PATH, onpath);

}

