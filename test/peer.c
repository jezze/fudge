#include <fudge.h>
#include <abi.h>

static void ondata(struct message *message)
{

    channel_forward(0, message->source, message->type, message);

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_INFO, ondata);

}

