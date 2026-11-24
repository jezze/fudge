#include <fudge.h>
#include <abi.h>

static void output(unsigned int source, struct message *message)
{

    channel_forward(0, source, MESSAGE_TYPE_DATA, message);

}

static void ondata(struct message *message)
{

    output(message->source, message);

}

static void onpath(struct message *message)
{

    fs_read_each(1, message->source, message->data, output);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

